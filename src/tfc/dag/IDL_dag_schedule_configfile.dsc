STRUCT	dag_schedule_configfile
{
	STRUCT	schedule
	{
		STRING	64	schedule_name
		STRING	256	schedule_desc
	}
	STRUCT	batches
	{
		STRUCT	batches_info	ARRAY	1000
		{
			STRING	64	batch_name
			STRING	256	batch_desc
			INT	4	view_pos_x
			INT	4	view_pos_y
			STRUCT	tasks	ARRAY	100
			{
				STRING	256	program_and_params
				INT	4	timeout
			}
		}
		STRUCT	batches_direction	ARRAY	1000
		{
			STRING	64	from_batch
			STRING	64	to_batch
		}
	}
}

