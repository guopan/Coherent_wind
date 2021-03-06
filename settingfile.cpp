﻿#include "settingfile.h"

#include <QSettings>
#include <QDebug>
#include <QtCore>
#include "Acqsettings.h"

settingFile::settingFile()
{
}

void settingFile::init_fsetting(const ACQSETTING &setting)
{
	fsetting = setting;
	qDebug() << "fsetting chuan di";
}

void settingFile::writeTo_file(const ACQSETTING &setting,QString a )
{
	fsetting = setting;

	QSettings settings(a,QSettings::IniFormat);
	settings.beginGroup("Laser_parameters");
	settings.setValue("laserRPF",fsetting.laserRPF);					//激光重频
	settings.setValue("laserPulseWidth",fsetting.laserPulseWidth);		//激光脉宽
	settings.setValue("laserWaveLength",fsetting.laserWaveLength);		//激光波长
	settings.setValue("AOM_Freq",fsetting.AOM_Freq);					//AOM移频量
	settings.endGroup();

	settings.beginGroup("Scan_parameters");
	settings.setValue("elevationAngle",fsetting.elevationAngle);		//俯仰角
	settings.setValue("start_azAngle",fsetting.start_azAngle);			//起始角
	settings.setValue("step_azAngle",fsetting.step_azAngle);			//步进角
	settings.setValue("angleNum",fsetting.angleNum);					//方向数
	settings.setValue("circleNum",fsetting.circleNum);					//圆周数
	settings.setValue("anglekey",fsetting.anglekey);					//方向数
	settings.setValue("circlekey",fsetting.circlekey);					//圆周数
	settings.endGroup();

	settings.beginGroup("Sample_parameters");
	settings.setValue("singleCh",fsetting.singleCh);					//单通道
	settings.setValue("doubleCh",fsetting.doubleCh);					//双通道
	settings.setValue("triggerLevel",fsetting.triggleLevel);			//触发电平
	settings.setValue("sampleFreq",fsetting.sampleFreq);				//采样频率
	settings.setValue("detRange",fsetting.detRange);					//探测距离
	settings.setValue("sampleNum",fsetting.sampleNum);
	settings.setValue("plsAccNum",fsetting.plsAccNum);					//脉冲数
	settings.endGroup();

	settings.beginGroup("File_store");
	settings.setValue("DatafilePath",fsetting.DatafilePath);					//文件保存路径
	settings.setValue("autocreate_datafile",fsetting.autocreate_datafile);		//自动创建日期文件夹
	settings.setValue("dataFileNamePrefix",fsetting.dataFileName_Prefix);		//前缀文件名
	settings.setValue("dataFileNameSuffix",fsetting.dataFileName_Suffix);		//后缀文件名
	settings.endGroup();
}

void settingFile::readFrom_file(QString a)
{
	QSettings settings(a,QSettings::IniFormat);
	settings.beginGroup("Laser_parameters");
	fsetting.laserRPF = settings.value("laserRPF").toInt();
	qDebug() << "fsetting.laserRPF = " << fsetting.laserRPF;
	fsetting.laserPulseWidth = settings.value("laserPulseWidth").toInt();
	fsetting.laserWaveLength = settings.value("laserWaveLength").toInt();
	fsetting.AOM_Freq = settings.value("AOM_Freq").toInt();
	settings.endGroup();

	settings.beginGroup("Scan_parameters");
	fsetting.elevationAngle = settings.value("elevationAngle").toInt();		//俯仰角
	fsetting.start_azAngle = settings.value("start_azAngle").toInt();		//起始角
	fsetting.step_azAngle = settings.value("step_azAngle").toInt();			//步进角
	fsetting.angleNum = settings.value("angleNum").toInt();					//方向数
	fsetting.circleNum = settings.value("circleNum").toFloat();				//圆周数
	fsetting.anglekey = settings.value("anglekey").toBool();				//方向键
	fsetting.circlekey = settings.value("circlekey").toBool();				//圆周键
	settings.endGroup();

	settings.beginGroup("Sample_parameters");
	fsetting.singleCh = settings.value("singleCh").toBool();				//单通道
	fsetting.doubleCh = settings.value("doubleCh").toBool();				//双通道
	fsetting.triggleLevel = settings.value("triggerLevel").toInt();			//触发电平
	fsetting.sampleFreq = settings.value("sampleFreq").toInt();				//采样频率
	fsetting.sampleNum = settings.value("sampleNum").toInt();
	fsetting.detRange = settings.value("detRange").toFloat();				//探测距离
	fsetting.plsAccNum = settings.value("plsAccNum").toInt();				//脉冲数
	settings.endGroup();

	settings.beginGroup("File_store");
	fsetting.DatafilePath = settings.value("DatafilePath").toString();					//文件保存路径
	fsetting.autocreate_datafile = settings.value("autocreate_datafile").toBool();		//自动创建最小文件夹
	fsetting.dataFileName_Prefix = settings.value("dataFileNamePrefix").toString();		//前缀文件名
	fsetting.dataFileName_Suffix = settings.value("dataFileNameSuffix").toString();		//后缀文件名
	settings.endGroup();
}

void settingFile::checkValid()
{
}

//检查settings.ini是否存在，若不存在则创建
void settingFile::test_create_file(QString a)
{
	QFileInfo file(a);
	if(file.exists() == false)
	{
		QSettings settings(a,QSettings::IniFormat);
		settings.beginGroup("Laser_parameters");
		settings.setValue("laserRPF",10000);				//激光重频
		settings.setValue("laserPulseWidth",140);			//激光脉宽
		settings.setValue("laserWaveLength",20);			//激光波长
		settings.setValue("AOM_Freq",120);					//AOM移频量
		settings.endGroup();

		settings.beginGroup("Scan_parameters");
		settings.setValue("elevationAngle",60);				//俯仰角
		settings.setValue("start_azAngle",0);				//起始角
		settings.setValue("step_azAngle",90);				//步进角
		settings.setValue("angleNum",80);					//方向数
		settings.setValue("circleNum",20);					//圆周数
		settings.setValue("anglekey",true);					//方向键
		settings.setValue("circlekey",false);				//圆周键
		settings.endGroup();

		settings.beginGroup("Sample_parameters");
		settings.setValue("singleCh",true);					//单通道
		settings.setValue("doubleCh",false);				//双通道
		settings.setValue("triggerLevel",1);				//触发电平
		settings.setValue("sampleFreq",550);				//采样频率
		settings.setValue("detRange",6000);					//探测距离
		settings.setValue("sampleNum",22000);
		settings.setValue("plsAccNum",100);					//脉冲数
		settings.endGroup();

		settings.beginGroup("File_store");
		settings.setValue("DatafilePath","");				//文件保存路径
		settings.setValue("autocreate_datafile",true);		//自动创建日期文件夹
		settings.setValue("dataFileNamePrefix","");			//前缀文件名
		settings.setValue("dataFileNameSuffix","001");		//后缀文件名
		settings.endGroup();
	}
	else
		qDebug() <<"Settings file exist";
}

bool settingFile::isSettingsChanged(const ACQSETTING &setting)
{
	ACQSETTING dlgsetting = setting;
	if(fsetting.laserRPF != dlgsetting.laserRPF)								//激光重频
		return true;
	qDebug() << "151";
	if(fsetting.laserPulseWidth != dlgsetting.laserPulseWidth)					//脉冲宽度
		return true;
	qDebug() << "154";
	if(fsetting.laserWaveLength != dlgsetting.laserWaveLength)					//激光波长
		return true;
	qDebug() << "157";
	if(fsetting.AOM_Freq != dlgsetting.AOM_Freq)								//AOM移频量
		return true;
	qDebug() << "160";

	if(fsetting.elevationAngle != dlgsetting.elevationAngle)					//俯仰角
		return true;
	qDebug() << "164";
	if(fsetting.start_azAngle != dlgsetting.start_azAngle)						//起始角
		return true;
	qDebug() << "167";
	if(fsetting.step_azAngle != dlgsetting.step_azAngle)						//步进角
		return true;
	qDebug() << "170";
	if(fsetting.angleNum != dlgsetting.angleNum)								//方向数
		return true;
	qDebug() << "173";
	if(fsetting.circleNum != dlgsetting.circleNum)								//圆周数
		return true;
	qDebug() << "176";
	if(fsetting.anglekey != dlgsetting.anglekey)
		return true;
	qDebug() << "179";
	if(fsetting.circlekey != dlgsetting.circlekey)
		return true;
	qDebug() << "182";

	if(fsetting.singleCh != dlgsetting.singleCh)								//单通道
		return true;
	qDebug() << "186";
	if(fsetting.doubleCh != dlgsetting.doubleCh)								//双通道
		return true;
	qDebug() << "189";
	if(fsetting.triggleLevel != dlgsetting.triggleLevel)						//触发电平
		return true;
	qDebug() << "192";
	if(fsetting.sampleFreq != dlgsetting.sampleFreq)							//采样频率
		return true;
	qDebug() << "195";
	if(fsetting.detRange != dlgsetting.detRange)								//探测距离
		return true;
	qDebug() << "198";
	if(fsetting.sampleNum != dlgsetting.sampleNum)
		return true;
	qDebug() << "201";
	if(fsetting.plsAccNum != dlgsetting.plsAccNum)
		return true;
	qDebug() << "204";

	if(fsetting.DatafilePath != dlgsetting.DatafilePath)						//文件保存路径
		return true;
	qDebug() << "208";
	if(fsetting.autocreate_datafile != dlgsetting.autocreate_datafile)			//自动创建日期文件夹
		return true;
	qDebug() << "211";
	if(fsetting.dataFileName_Prefix != dlgsetting.dataFileName_Prefix)
		return true;
	qDebug() << "214";
	if(fsetting.dataFileName_Suffix != dlgsetting.dataFileName_Suffix)
		return true;
	qDebug() << "217";

	return false;
}

ACQSETTING settingFile::get_setting()
{
	return fsetting;
}

