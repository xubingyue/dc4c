update pm_hzbat_schedule set begin_datetime='',end_datetime='',progress=0;
update pm_hzbat_batches_info set begin_datetime='',end_datetime='',progress=0,pretask_ip='',pretask_port=0,pretask_progress=0,pretask_error=0,pretask_status=0;
delete from pm_hzbat_batches_tasks;

