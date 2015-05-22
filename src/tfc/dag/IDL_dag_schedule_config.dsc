STRUCT	dag_schedule_config
{
	STRUCT	schedule
	{
		STRING	64	schedule_name
		STRING	256	schedule_desc
	}
	STRUCT	batches
	{
		STRUCT	batches_direction	ARRAY	100
		{
			STRING	64	from
			STRING	64	to
		}
		STRUCT	batches_info	ARRAY	100
		{
			STRING	64	name
			STRING	256	desc
			INT	4	view_pos_x
			INT	4	view_pos_y
			STRUCT	tasks	ARRAY	100
			{
				STRING	256	program_and_params
				INT	4	timeout
			}
		}
	}
}

