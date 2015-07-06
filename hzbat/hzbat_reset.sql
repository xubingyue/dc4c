delete from pm_hzbat_batches_tasks;

update pm_hzbat_schedule set begin_datetime='',end_datetime='',progress=0;
update pm_hzbat_batches_info set begin_datetime='',end_datetime='',progress=0,pretask_progress=0,pretask_errno=0;pretask_status=0;
update pm_hzbat_batches_tasks set begin_datetime='',end_datetime='',progress=0,error=0,status=0;

