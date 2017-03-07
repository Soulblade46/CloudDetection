#include <ocl_boiler.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	rcl_int err;

	cl_int numels;
	size_t lws = 0;

	if (getenv("VECSIZE")) {
		vecsize = atoi(getenv("VECSIZE"));
	} else {
		vecsize = 4;
	}

	if (argc > 1) {
		numels = atoi(argv[1]);
	} else {
		fprintf(stderr, "inserire numero di elementi\n");
		exit(1);
	}

	// vettorializzazione: assicuriamoci che sia multiplo del
	// numero di elementi nel vettore
	numels = round_mul_up(numels, 4);

	if (argc > 2) {
		lws = atoi(argv[2]);
	}

	char compile_opt[BUFSIZE];
	snprintf(compile_opt, BUFSIZE, "-DVECTYPE=int%zu", vecsize);

	cl_platform_id piattaforma = select_platform();
	cl_device_id device = select_device(piattaforma);
	cl_context ctx = create_context(piattaforma, device);
	cl_command_queue que = create_queue(ctx, device);
	cl_program devcode = create_program(
		"vecsum.ocl", compile_opt,
		ctx, device);
	cl_mem a, b, c;
	size_t numbytes = numels*sizeof(cl_int);
	a = clCreateBuffer(ctx,
		CL_MEM_READ_WRITE,
		numbytes, NULL,
		&err);
	ocl_check(err, "create a");
	b = clCreateBuffer(ctx,
		CL_MEM_READ_WRITE,
		numbytes, NULL,
		&err);
	ocl_check(err, "create b");
	c = clCreateBuffer(ctx,
		CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
		numbytes, NULL,
		&err);
	ocl_check(err, "create c");

	cl_kernel init_k = clCreateKernel(devcode, "init", &err);
	ocl_check(err, "create init");

	cl_kernel sum_k = clCreateKernel(devcode, "sum", &err);
	ocl_check(err, "create sum");

	cl_kernel sum_vec_k = clCreateKernel(devcode, "sum_vec", &err);
	ocl_check(err, "create sum");

	err = clGetKernelWorkGroupInfo(init_k, device,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_init_wg), &preferred_init_wg, NULL);
	printf("preferred init work-group size multiple: %zu\n",
		preferred_init_wg);
	if (!preferred_init_wg) {
		puts("non sembra un buon numero, usiamo 32");
		preferred_init_wg = 32;
	}

	err = clGetKernelWorkGroupInfo(sum_k, device,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_sum_wg), &preferred_sum_wg, NULL);
	printf("preferred sum work-group size multiple: %zu\n",
		preferred_sum_wg);
	if (!preferred_sum_wg) {
		puts("non sembra un buon numero, usiamo 32");
		preferred_sum_wg = 32;
	}

	err = clGetKernelWorkGroupInfo(sum_vec_k, device,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_sum_vec_wg), &preferred_sum_vec_wg, NULL);
	printf("preferred sum_vec work-group size multiple: %zu\n",
		preferred_sum_vec_wg);
	if (!preferred_sum_vec_wg) {
		puts("non sembra un buon numero, usiamo 32");
		preferred_sum_vec_wg = 32;
	}

	cl_event init_evt = init(que, init_k,
		a, b, numels,
		lws,
		0, NULL);
	cl_event sum_evt = sum(que, sum_k,
		c, a, b, numels,
		lws,
		1, &init_evt);
	cl_event sum_vec_evt = sum_vec(que, sum_vec_k,
		c, a, b, numels,
		lws,
		1, &init_evt);

	cl_event map_c;
	cl_int * host_c = clEnqueueMapBuffer(que, c,
		CL_FALSE, CL_MAP_READ,
		0, numbytes,
		1, &sum_vec_evt, &map_c,
		&err);
	ocl_check(err, "map c");

	err = clFinish(que);
	ocl_check(err, "finish que");

	for (int i = 0; i < numels; ++i) {
		if (host_c[i] != numels) {
			printf("%u\t%u != %u\n", i, host_c[i], numels);
			exit(1);
		}
	}

	cl_event unmap_c;
	err = clEnqueueUnmapMemObject(que, c, host_c,
		0, NULL, &unmap_c);
	ocl_check(err, "unmap a");

	err = clFinish(que);
	ocl_check(err, "finish unmap");

	cl_ulong rt_ns;
	rt_ns = runtime_ns(init_evt);
	printf("init: %gms %gGB/s\n", rt_ns/(1.e6),
		2.0*numbytes/rt_ns);
	rt_ns = runtime_ns(sum_evt);
	printf("sum: %gms %gGB/s\n", rt_ns/(1.e6),
		3.0*numbytes/rt_ns);
	rt_ns = runtime_ns(sum_vec_evt);
	printf("sum_vec: %gms %gGB/s\n", rt_ns/(1.e6),
		3.0*numbytes/rt_ns);
	rt_ns = runtime_ns(map_c);
	printf("map c: %gms %gGB/s\n", rt_ns/(1.e6),
		1.0*numbytes/rt_ns);
	rt_ns = runtime_ns(unmap_c);
	printf("unmap c: %gms %gGB/s\n", rt_ns/(1.e6),
		1.0*numbytes/rt_ns);

	clReleaseMemObject(c);
	clReleaseMemObject(b);
	clReleaseMemObject(a);
}