STRUCT	execute_program_request
{
	STRING	40	ip
	INT	4	port
	STRING	20	tid
	STRING	256	program_and_params
	STRING	32	program_md5_exp
	INT	4	timeout
	INT	1	bind_cpu_flag
}

