STRUCT	worker_notice_request
{
	STRING	64	sysname
	STRING	64	release
	INT	4	bits
	
	STRING	40	ip
	INT	4	port
	
	INT	4	is_working
	STRING	256	program_and_params
}

