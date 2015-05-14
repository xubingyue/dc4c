STRUCT	query_workers_response
{
	INT	4	response_code
	STRUCT	nodes	ARRAY	100
	{
		STRUCT	node
		{
			STRING	40	ip
			INT	4	port
		}
	}
}

