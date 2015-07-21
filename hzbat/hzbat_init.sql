set names gb2312 ;

-- ----------------------------------------------------------------------------
-- 清理所有数据
-- ----------------------------------------------------------------------------

truncate table pm_hzbat_schedule ;
truncate table pm_hzbat_batches_info ;
truncate table pm_hzbat_batches_filter ;
truncate table pm_hzbat_batches_tasks ;
truncate table pm_hzbat_batches_direction ;

-- ----------------------------------------------------------------------------
-- 插入计划表数据
-- ----------------------------------------------------------------------------

INSERT INTO pm_hzbat_schedule VALUES (10,'POIT_STAD','数据标准化','','',0);
INSERT INTO pm_hzbat_schedule VALUES (20,'POIT_CALC','积分计算','','',0);
INSERT INTO pm_hzbat_schedule VALUES (25,'POIT_DAYE','日切','','',0);
INSERT INTO pm_hzbat_schedule VALUES (30,'POIT_RPT','报表','','',0);

-- ----------------------------------------------------------------------------
-- 插入批量信息表数据
-- ----------------------------------------------------------------------------

-- 数据标准化计划

INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHA901','客户信息同步',0,0,1,'','',0,'POTHA901 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB001','存款类到标准数据',0,0,1,'','',0,'POTHB001 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB002','信贷类到标准数据',0,0,1,'','',0,'POTHB002 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB003','基金数据到标准数据',0,0,1,'','',0,'POTHB003 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB004','黄金T+D数据到标准数据',0,0,1,'','',0,'POTHB004 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB005','电子国债到标准数据',0,0,1,'','',0,'POTHB005 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB006','网银数据到标准数据',0,0,1,'','',0,'POTHB006 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB007','借记卡数据到标准数据',0,0,1,'','',0,'POTHB007 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB008','渠道类数据到标准数据',0,0,1,'','',0,'POTHB008 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB009','微信银行数据到标准数据',0,0,1,'','',0,'POTHB009 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB010','借记卡pos消费数据到标准数据',0,0,1,'','',0,'POTHB010 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB011','凭证式国债到标准数据',0,0,1,'','',0,'POTHB011 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB012','电子式国债到标准数据',0,0,1,'','',0,'POTHB012 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB013','理财签约到标准数据',0,0,1,'','',0,'POTHB013 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB014','信贷类到标准数据',0,0,1,'','',0,'POTHB014 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB015','理财和基金到标准数据',0,0,1,'','',0,'POTHB015 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB016','信用卡pos消费到标准数据',0,0,1,'','',0,'POTHB016 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB017','代理类到标准数据',0,0,1,'','',0,'POTHB017 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB018','代发工资签约数据标准化',0,0,1,'','',0,'POTHB018 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB019','结构性存款',0,0,1,'','',0,'POTHB019 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB020','个人自动转存',0,0,1,'','',0,'POTHB020 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB021','资金归集签约',0,0,1,'','',0,'POTHB021 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB022','微贷卡网银签约',0,0,1,'','',0,'POTHB022 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB023','代扣业务签约',0,0,1,'','',0,'POTHB023 -s',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB024','我行POS机具交易量',0,0,1,'','',0,'POTHB024 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB025','基金按购买金额送积分到标准',0,0,1,'','',0,'POTHB025 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB026','在线缴费按笔数送积分',0,0,1,'','',0,'POTHB026 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB027','信用卡还款',0,0,1,'','',0,'POTHB027 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB028','自考报名',0,0,1,'','',0,'POTHB028 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB029','学费缴费按笔数送积分',0,0,1,'','',0,'POTHB029 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB030','理财购买按金额送积分',0,0,1,'','',0,'POTHB030 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_STAD','POTHB031','资金归集成功送积分',0,0,1,'','',0,'POTHB031 -s',60,'',0,0,0,0);

-- 积分计算计划

INSERT INTO pm_hzbat_batches_info VALUES ('POIT_CALC','POTHC001','积分计算',0,0,1,'','',0,'POTHC001 -m',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_CALC','POTHC101','积分汇总',0,0,1,'','',0,'POTHC101 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_CALC','POTHC201','积分汇总',0,0,1,'','',0,'POTHC201 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_CALC','POTHC301','积分入账',0,0,1,'','',0,'POTHC301 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_CALC','POTHC302','时点积分余额处理',0,0,1,'','',0,'POTHC302 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_CALC','POTHC401','积分失效',0,0,1,'','',0,'POTHC401 -s',60,'',0,0,0,0);

-- 积分计算计划

INSERT INTO pm_hzbat_batches_info VALUES ('POIT_DAYE','POTHD001','业务日期切换',0,0,1,'','',0,'POTHD001 -s',60,'',0,0,0,0);

-- 报表生成计划

INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR001','积分账号月汇总',0,0,1,'','',0,'POTHR001 -m',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR002','项目明细每日汇总',0,0,1,'','',0,'POTHR002 -m',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR003','客户项目明细每日TOP',0,0,1,'','',0,'POTHR003 -m',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR101','客户积分产生日报表',0,0,1,'','',0,'POTHR101 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR201','客户积分产生月报表',0,0,1,'','',0,'POTHR201 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR301','客户积分产生季报表',0,0,1,'','',0,'POTHR301 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR401','客户积分产生年报表',0,0,1,'','',0,'POTHR401 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR102','条线积分产生日报表',0,0,1,'','',0,'POTHR102 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR202','条线积分产生月报表',0,0,1,'','',0,'POTHR202 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR302','条线积分产生季报表',0,0,1,'','',0,'POTHR302 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR402','条线积分产生年报表',0,0,1,'','',0,'POTHR402 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR103','条线客户积分产生日报表',0,0,1,'','',0,'POTHR103 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR203','条线客户积分产生月报表',0,0,1,'','',0,'POTHR203 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR303','条线客户积分产生季报表',0,0,1,'','',0,'POTHR303 -s',60,'',0,0,0,0);
INSERT INTO pm_hzbat_batches_info VALUES ('POIT_RPT','POTHR403','条线客户积分产生年报表',0,0,1,'','',0,'POTHR403 -s',60,'',0,0,0,0);

-- ----------------------------------------------------------------------------
-- 插入批量过滤表数据
-- ----------------------------------------------------------------------------

INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_CALC','POTHC302','DD','ME');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR201','DD','ME');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR202','DD','ME');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR203','DD','ME');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR301','MM-DD','03-31,06-30,09-30,12-31');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR302','MM-DD','03-31,06-30,09-30,12-31');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR303','MM-DD','03-31,06-30,09-30,12-31');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR401','MM-DD','12-31');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR402','MM-DD','12-31');
INSERT INTO pm_hzbat_batches_filter VALUES ('POIT_RPT','POTHR403','MM-DD','12-31');

-- ----------------------------------------------------------------------------
-- 插入批量依赖表数据
-- ----------------------------------------------------------------------------

-- 数据标准化计划

INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHA901');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB001');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB002');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB003');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB004');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB005');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB006');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB007');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB008');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB009');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB010');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB011');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB012');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB013');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB014');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB015');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB016');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB017');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB018');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB019');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB020');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB021');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB022');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB023');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB024');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB025');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB026');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB027');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB028');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB029');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB030');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','','POTHB031');

INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHA901','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB001','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB002','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB003','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB004','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB005','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB006','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB007','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB008','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB009','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB010','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB011','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB012','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB013','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB014','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB015','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB016','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB017','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB018','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB019','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB020','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB021','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB022','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB023','');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB024','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB025','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB026','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB027','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB028','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB029','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB030','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_STAD','POTHB031','');

-- 积分计算计划

INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_CALC','','POTHC001');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_CALC','POTHC001','POTHC101');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_CALC','POTHC101','POTHC201');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_CALC','POTHC201','POTHC301');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_CALC','POTHC301','POTHC302');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_CALC','POTHC302','POTHC401');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_CALC','POTHC401','');

-- 积分计算计划

INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_DAYE','','POTHD001');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_DAYE','POTHD001','');

-- 报表生成计划

INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','','POTHR001');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR001','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','','POTHR002');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR002','POTHR003');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR003','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','','POTHR101');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR101','POTHR201');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR201','POTHR301');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR301','POTHR401');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR401','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','','POTHR102');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR102','POTHR202');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR202','POTHR302');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR302','POTHR402');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR402','');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','','POTHR103');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR103','POTHR203');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR203','POTHR303');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR303','POTHR403');
INSERT INTO pm_hzbat_batches_direction VALUES ('POIT_RPT','POTHR403','');

-- ----------------------------------------------------------------------------
-- 插入任务表数据
-- ----------------------------------------------------------------------------

-- ----------------------------------------------------------------------------
-- 测试用小计划
-- ----------------------------------------------------------------------------

-- INSERT INTO pm_hzbat_schedule VALUES (-5,'TEST','测试','','',0);

-- INSERT INTO pm_hzbat_batches_info VALUES ('TEST','TEST1','测试1',0,0,1,'','',0,'dc4c_test_worker_sleep 1',60,'',0,0,0,0);
-- INSERT INTO pm_hzbat_batches_info VALUES ('TEST','TEST2','测试2',0,0,1,'','',0,'dc4c_test_worker_sleep 2',60,'',0,0,0,0);

-- INSERT INTO pm_hzbat_batches_direction VALUES ('TEST','','TEST1');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('TEST','TEST1','TEST2');
-- INSERT INTO pm_hzbat_batches_direction VALUES ('TEST','TEST2','');

