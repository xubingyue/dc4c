STRUCT	query_workers_response
{
	INT	4	response_code
	INT	4	count
	STRUCT	nodes	ARRAY	3
	{
		STRUCT	node
		{
			STRING	40	ip
			INT	4	port
		}
	}
}

