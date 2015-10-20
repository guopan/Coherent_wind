﻿#include "paradialog.h"
#include "ui_paradialog.h"
#include "Acqsettings.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include <QString>
#include <QDir>

#define c 3														//光速

paraDialog::paraDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::paraDialog)
{
	ui->setupUi(this);
}

paraDialog::~paraDialog()
{
	delete ui;
}

void paraDialog::init_setting(const ACQSETTING &setting)
{
	psetting = setting;
	defaulsetting = setting;
	dlg_setfile.init_fsetting(psetting);								//把psetting传递给fsetting
}

void paraDialog::initial_para()
{
	update_show();

	connect(ui->lineEdit_elevationAngle,&QLineEdit::textChanged,this,&paraDialog::set_elevationAngle);			//俯仰角 -> 探测方式
	connect(ui->lineEdit_step_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_step_azAngle);				//步进角 -> 探测方式
	connect(ui->lineEdit_step_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_circleNum);					//步进角 -> 方向数
	connect(ui->lineEdit_step_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_angleNum);					//步进角 -> 圆周数

	connect(ui->lineEdit_circleNum, &QLineEdit::textChanged,this,&paraDialog::set_angleNum);					//圆周数 -> 方向数
	connect(ui->lineEdit_angleNum,&QLineEdit::textChanged,this,&paraDialog::set_circleNum);						//方向数 -> 圆周数
	connect(ui->radioButton_anglekey,&QRadioButton::clicked,this,&paraDialog::set_anglekey);					//方向键
	connect(ui->radioButton_circlekey,&QRadioButton::clicked,this,&paraDialog::set_circlekey);					//圆周键

	connect(ui->radioButton_singleCh,&QRadioButton::clicked,this,&paraDialog::set_singleCh);					//单通道
	connect(ui->radioButton_doubleCh,&QRadioButton::clicked,this,&paraDialog::set_doubleCh);					//双通道
	connect(ui->comboBox_sampleFreq,&QComboBox::currentTextChanged,this,&paraDialog::set_sampleFreq);			//采样频率
	connect(ui->lineEdit_detRange,&QLineEdit::textChanged,this,&paraDialog::set_detRange);						//探测距离
	connect(ui->lineEdit_plsAccNum,&QLineEdit::textChanged,this,&paraDialog::set_plsAccNum);					//脉冲数
	connect(ui->lineEdit_dataFileName_Suffix,&QLineEdit::textChanged,this,&paraDialog::set_dataFileName_Suffix);//后缀名

	connect(ui->lineEdit_laserRPF,&QLineEdit::textChanged,this,&paraDialog::set_laserRPF);						//激光重频
	connect(ui->lineEdit_laserPulseWidth,&QLineEdit::textChanged,this,&paraDialog::set_laserPulseWidth);		//激光脉宽
	connect(ui->lineEdit_laserWaveLength,&QLineEdit::textChanged,this,&paraDialog::set_laserWaveLength);		//激光波长
	connect(ui->lineEdit_AOM_Freq,&QLineEdit::textChanged,this,&paraDialog::set_AOM_Freq);						//AOM移频量
	connect(ui->lineEdit_start_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_start_azAngle);			//起始角
	connect(ui->lineEdit_triggerLevel,&QLineEdit::textChanged,this,&paraDialog::set_triggerLevel);				//触发电平

	connect(ui->lineEdit_sampleNum,&QLineEdit::textChanged,this,&paraDialog::set_filesize);						//采样点数
}

void paraDialog::set_elevationAngle()												//俯仰角 决定探测方向是水平还是径向
{
	psetting.elevationAngle = ui->lineEdit_elevationAngle->text().toInt();
	show_detect_mode();																//参考信息中的探测方式
}

void paraDialog::set_step_azAngle()													//步进角 是否为0决定是不是圆周扫描，大小决定每周扫描数//psetting获取编辑框值
{
	psetting.step_azAngle = ui->lineEdit_step_azAngle->text().toInt();
	show_detect_mode();

	if(psetting.step_azAngle == 0)
		ui->groupBox_2->setEnabled(false);											//扫描探测
	else
		ui->groupBox_2->setEnabled(true);
}

//设置方向数输入框的显示数值
void paraDialog::set_angleNum()														//方向数 决定圆周数，影响总数据量（双通道乘2），探测总时间//psetting获取编辑框值
{
	if(psetting.anglekey == true)
		psetting.angleNum = ui->lineEdit_angleNum->text().toInt();
	else
	{
		psetting.circleNum = ui->lineEdit_circleNum->text().toFloat();
		psetting.step_azAngle = ui->lineEdit_step_azAngle->text().toInt();
		psetting.angleNum = (int)(psetting.circleNum*360/psetting.step_azAngle);
		ui->lineEdit_angleNum->setText(QString::number(psetting.angleNum));
	}

	ui->lineEdit_totalTime->setText(QString::number(10*psetting.angleNum));			//总时间
	if(ui->radioButton_singleCh->isChecked())										//总数据量
		ui->lineEdit_totalsize->setText(QString::number(psetting.angleNum*direct_size/(1024*1024),'f',2));
	else
		ui->lineEdit_totalsize->setText(QString::number(2*psetting.angleNum*direct_size/(1024*1024),'f',2));
}

//设置圆周数输入框的显示数值
void paraDialog::set_circleNum()													//圆周数 影响方向数,psetting获取编辑框值
{
	if(psetting.circlekey == true)
		psetting.circleNum = ui->lineEdit_circleNum->text().toFloat();
	else
	{
		psetting.angleNum = ui->lineEdit_angleNum->text().toInt();
		psetting.step_azAngle = ui->lineEdit_step_azAngle->text().toInt();
		psetting.circleNum = (float)psetting.angleNum/360*psetting.step_azAngle;
		ui->lineEdit_circleNum->setText(QString::number(psetting.circleNum,'f',2));
	}
}

void paraDialog::set_anglekey()														//方向键
{
	psetting.anglekey = true;
	psetting.circlekey = false;
	ui->lineEdit_angleNum->setEnabled(true);
	ui->lineEdit_circleNum->setEnabled(false);
}

void paraDialog::set_circlekey()													//圆周键
{
	psetting.anglekey = false;
	psetting.circlekey = true;
	ui->lineEdit_angleNum->setEnabled(false);
	ui->lineEdit_circleNum->setEnabled(true);
}

void paraDialog::set_singleCh()														//单通道 影响触发电平，以及ch1的文件名编辑框、数据量
{
	psetting.singleCh = true;
	psetting.doubleCh = false;
	ui->lineEdit_triggerLevel->setEnabled(true);
	updates_filename();
	ui->lineEdit_sglfilesize->setText(QString::number(direct_size/(1024*1024),'f',2));
	ui->lineEdit_totalsize->setText(QString::number(psetting.angleNum*direct_size/(1024*1024),'f',2));
}

void paraDialog::set_doubleCh()														//双通道 影响触发电平，以及chA、B文件名编辑框数据量
{
	psetting.singleCh = false;
	psetting.doubleCh = true;
	ui->lineEdit_triggerLevel->setEnabled(false);
	updated_filename();
	ui->lineEdit_sglfilesize->setText(QString::number(2*direct_size/(1024*1024),'f',2));
	ui->lineEdit_totalsize->setText(QString::number(2*psetting.angleNum*direct_size/(1024*1024),'f',2));
}

void paraDialog::set_sampleFreq()													//采样频率 影响采样点数、单文件量、总数据量//psetting获取编辑框值
{
	psetting.sampleFreq = ui->comboBox_sampleFreq->currentText().toInt();
	psetting.sampleNum = psetting.sampleFreq*2*psetting.detRange/300;
	direct_size = 48+psetting.plsAccNum*psetting.sampleNum*2;						//单个方向上的数据量

	ui->lineEdit_sampleNum->setText(QString::number(psetting.sampleNum));			//采样点数
}

void paraDialog::set_detRange()														//探测距离 影响采样点数、单文件量、总数据量//psetting获取编辑框值
{
	psetting.detRange = 1000*(ui->lineEdit_detRange->text().toFloat());
	psetting.sampleNum = psetting.sampleFreq*2*psetting.detRange/300;
	direct_size = 48+psetting.plsAccNum*psetting.sampleNum*2;						//单个方向上的数据量

	ui->lineEdit_sampleNum->setText(QString::number(psetting.sampleNum));			//采样点数
}

void paraDialog::set_filesize()														//参考信息中的单文件量和总数据量
{
	if(ui->radioButton_singleCh->isChecked())
	{
		ui->lineEdit_sglfilesize->setText(QString::number(direct_size/(1024*1024),'f',2));
		ui->lineEdit_totalsize->setText(QString::number(psetting.angleNum*direct_size/(1024*1024),'f',2));
	}
	else
	{
		ui->lineEdit_sglfilesize->setText(QString::number(2*direct_size/(1024*1024),'f',2));
		ui->lineEdit_totalsize->setText(QString::number(2*psetting.angleNum*direct_size/(1024*1024),'f',2));
	}
}

void paraDialog::set_plsAccNum()													//脉冲数影响单文件量、总数据量（双通道乘2）//psetting获取编辑框值
{
	psetting.plsAccNum = ui->lineEdit_plsAccNum->text().toInt();
	direct_size = 48+psetting.plsAccNum*psetting.sampleNum*2;
	set_filesize();
}

void paraDialog::set_dataFileName_Suffix()											//文件的后缀名
{
	psetting.dataFileName_Suffix = ui->lineEdit_dataFileName_Suffix->text();
	if(ui->radioButton_singleCh->isChecked())
		updates_filename();
	else
		updated_filename();
}

void paraDialog::set_laserRPF()														//psetting获取编辑框值
{
	psetting.laserRPF = ui->lineEdit_laserRPF->text().toInt();
}

void paraDialog::set_laserPulseWidth()												//psetting获取编辑框值
{
	psetting.laserPulseWidth = ui->lineEdit_laserPulseWidth->text().toInt();
}

void paraDialog::set_laserWaveLength()												//psetting获取编辑框值
{
	psetting.laserWaveLength = ui->lineEdit_laserWaveLength->text().toInt();
}

void paraDialog::set_AOM_Freq()														//psetting获取编辑框值
{
	psetting.AOM_Freq = ui->lineEdit_AOM_Freq->text().toInt();
}

void paraDialog::set_start_azAngle()												//psetting获取编辑框值
{
	psetting.start_azAngle = ui->lineEdit_start_azAngle->text().toInt();
}

void paraDialog::set_triggerLevel()													//psetting获取编辑框值
{
	psetting.triggleLevel = ui->lineEdit_triggerLevel->text().toInt();
}

void paraDialog::on_pushButton_pathModify_clicked()									//修改路径键
{
	QFileDialog *fd=new QFileDialog(this,"Data File Path Choose",psetting.DatafilePath);
	fd->setFileMode(QFileDialog::Directory);
	fd->setOption(QFileDialog::ShowDirsOnly, true);
	if(fd->exec() == QFileDialog::Accepted)
	{
		QStringList file = fd->selectedFiles();
		QString str = static_cast<QString>(file.at(0));
		qDebug() << "Here I am" << str;

		if (str.length() == 3)
			str.resize(2);
		Set_DatafilePath(str);
	}
	on_checkBox_autocreate_datafile_clicked();
}

void paraDialog::Set_DatafilePath(QString str)										//路径显示设置
{
	QDir mypath(str);
	if(!mypath.exists())															//路径不存在，红色
	{
		ui->lineEdit_DatafilePath->setStyleSheet("color: red;"
												 "font-size:16px;"
												 "font-family:'微软雅黑';");
	}
	else																			//存在，黑色
	{
		ui->lineEdit_DatafilePath->setStyleSheet("color: black;"
												 "font-size:16px;"
												 "font-family:'微软雅黑';");
	}
	psetting.DatafilePath = str;
	ui->lineEdit_DatafilePath->setText(str);
}

void paraDialog::on_pushButton_sure_clicked()										//确定键
{
	if(dlg_setfile.isSettingsChanged(psetting))									//文件未保存时
	{
		QMessageBox::StandardButton reply = QMessageBox::warning(this,QString::fromLocal8Bit("提示"),
																 QString::fromLocal8Bit("修改的参数未保存，是否要保存修改"),
																 QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
		if(reply == QMessageBox::Save)												//点击是，弹出保存窗口
			on_pushButton_save_clicked();
		else
			if(reply == QMessageBox::Discard)										//点击否时，不保存并accept()
				accept();
	}
	else																			//文件若已保存，则accept()
		accept();
}

void paraDialog::on_pushButton_cancel_clicked()										//取消键
{
	reject();
}

void paraDialog::on_pushButton_save_clicked()										//保存键
{
	profile_path = QFileDialog::getSaveFileName(this,"另存",".","*.ini");
	if(!profile_path.isEmpty())
	{
		if(QFileInfo(profile_path).suffix().isEmpty())
			profile_path.append(".ini");											//如果无后缀，自动补上.ini
		qDebug() << "Profile_path = " << profile_path;
		dlg_setfile.writeTo_file(psetting,profile_path);
	}
}

void paraDialog::on_pushButton_load_clicked()										//加载键
{
	profile_path = QFileDialog::getOpenFileName(this,"打开",".","*.ini");
	if(!profile_path.isEmpty())
	{
		dlg_setfile.readFrom_file(profile_path);
		psetting = dlg_setfile.get_setting();
		defaulsetting = dlg_setfile.get_setting();
		update_show();
	}
}

void paraDialog::on_pushButton_reset_clicked()										//重置键
{
	psetting = defaulsetting;
	update_show();
}

ACQSETTING paraDialog::get_setting()
{
	return psetting;
}

void paraDialog::update_show()
{
	ui->lineEdit_laserRPF->setText(QString::number(psetting.laserRPF));
	ui->lineEdit_laserPulseWidth->setText(QString::number(psetting.laserPulseWidth));
	ui->lineEdit_laserWaveLength->setText(QString::number(psetting.laserWaveLength));
	ui->lineEdit_AOM_Freq->setText(QString::number(psetting.AOM_Freq));

	ui->lineEdit_elevationAngle->setText(QString::number(psetting.elevationAngle));
	ui->lineEdit_start_azAngle->setText(QString::number(psetting.start_azAngle));
	ui->lineEdit_step_azAngle->setText(QString::number(psetting.step_azAngle));
	ui->radioButton_anglekey->setChecked(psetting.anglekey);
	ui->radioButton_circlekey->setChecked(psetting.circlekey);
	ui->lineEdit_angleNum->setText(QString::number(psetting.angleNum));
	ui->lineEdit_circleNum->setText(QString::number(psetting.circleNum));

	ui->radioButton_singleCh->setChecked(psetting.singleCh);
	ui->radioButton_doubleCh->setChecked(psetting.doubleCh);
	ui->lineEdit_triggerLevel->setText(QString::number(psetting.triggleLevel));
	ui->comboBox_sampleFreq->setCurrentText(QString::number(psetting.sampleFreq));
	ui->lineEdit_detRange->setText(QString::number(psetting.detRange/1000));
	ui->lineEdit_sampleNum->setText(QString::number(psetting.sampleNum));					//初始采样点数
	ui->lineEdit_plsAccNum->setText(QString::number(psetting.plsAccNum));

	ui->lineEdit_DatafilePath->setText(psetting.DatafilePath);
	ui->checkBox_autocreate_datafile->setChecked(psetting.autocreate_datafile);
	ui->lineEdit_dataFileName_Suffix->setText(psetting.dataFileName_Suffix);
																							//下方参考信息
	show_detect_mode();																		//探测方式
	if(psetting.step_azAngle == 0)															//扫描探测box
		ui->groupBox_2->setEnabled(false);
	else
	{
		ui->groupBox_2->setEnabled(true);
		if(ui->radioButton_anglekey->isChecked())
		{
			ui->lineEdit_circleNum->setEnabled(false);
			ui->lineEdit_angleNum->setEnabled(true);
		}
		else
		{
			ui->lineEdit_circleNum->setEnabled(true);
			ui->lineEdit_angleNum->setEnabled(false);
		}
	}
	ui->lineEdit_totalTime->setText(QString::number(psetting.angleNum*10));					//初始探测时间

	QDateTime time = QDateTime::currentDateTime();
	psetting.dataFileName_Prefix = time.toString("yyyyMMdd");
	ui->lineEdit_dataFileName_Prefix->setText(psetting.dataFileName_Prefix);

	direct_size = 48+psetting.plsAccNum*psetting.sampleNum*2;
	if(ui->radioButton_singleCh->isChecked())
	{
		ui->lineEdit_triggerLevel->setEnabled(true);
		updates_filename();
		ui->lineEdit_sglfilesize->setText(QString::number(direct_size/(1024*1024),'f',2));
		ui->lineEdit_totalsize->setText(QString::number(psetting.angleNum*direct_size/(1024*1024),'f',2));
	}
	else
	{
		ui->lineEdit_triggerLevel->setEnabled(false);
		updated_filename();
		ui->lineEdit_sglfilesize->setText(QString::number(2*direct_size/(1024*1024),'f',2));
		ui->lineEdit_totalsize->setText(QString::number(2*psetting.angleNum*direct_size/(1024*1024),'f',2));
	}
}

void paraDialog::on_checkBox_autocreate_datafile_clicked()									//创建日期文件夹
{
	qDebug() << ui->checkBox_autocreate_datafile->isChecked();
	psetting.autocreate_datafile = ui->checkBox_autocreate_datafile->isChecked();
	QString str = psetting.DatafilePath;
	qDebug() << str;

	QDir mypath(str);
	QString dirname = mypath.dirName();
	qDebug() << dirname;
	QDateTime time = QDateTime::currentDateTime();

	if(ui->checkBox_autocreate_datafile->isChecked())
	{
		int num = dirname.toInt();
		int len = dirname.length();
		QString today_str = time.toString("yyyyMMdd");
		int today_int = today_str.toInt();
		if(len == 8 && (num != today_int) && qAbs(num - today_int)<10000)
		{
			str = mypath.absolutePath();
			int str_len = str.length();
			str.resize(str_len - 8);
			str += today_str;
			qDebug()<<str<<endl;
		}

		else if( dirname != time.toString("yyyyMMdd"))
		{
			str = mypath.absolutePath();
			str += QString("/");
			str += time.toString("yyyyMMdd");												//设置显示格式
			qDebug()<<"Dir not Match";
		}
		qDebug()<<str<<endl;
	}
	else																					//取消选择时，如果当前日期路径不存在，则取消，如存在，则不变。
	{
		if( dirname == time.toString("yyyyMMdd"))
		{
			if (!mypath.exists())
			{
				str = mypath.absolutePath();
				int str_len = str.length();
				str.resize(str_len - 9);													//减去/20xxxxxx
			}
			qDebug()<<"Dir Match"<<str<<endl;
		}
		//		mypath.mkpath(str);
	}
	Set_DatafilePath(str);
}

void paraDialog::on_pushButton_dataFileName_sch_clicked()									//自动查找最小序号
{
	QString filter_str = psetting.dataFileName_Prefix + "_ch[1AB]_";						//设置文件名过滤器，如"Prefix-[0123456789][0123456789][0123456789]"的形式
	int suffix_l = psetting.dataFileName_Suffix.length();
	for(int i=0;i<suffix_l;i++)
		filter_str += "[0123456789]";

	filter_str += ".wld";

	qDebug() << filter_str;
	QStringList FN_list;
	QStringList filter(filter_str);

	QDir *dir = new QDir(psetting.DatafilePath);											// 获取路径下的文件列表
	dir->setNameFilters(filter);

	QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));			// 设置文件名过滤器

	int file_numbers = fileInfo->count();													//搜索当前最大序号
	int max_num = 0;
	int tmp_num = 0;
	for(int i=0;i<file_numbers;i++)
	{
		FN_list<<fileInfo->at(i).baseName().right(suffix_l);
		tmp_num = fileInfo->at(i).baseName().right(suffix_l).toInt();
		if (tmp_num > max_num)
			max_num = tmp_num;
		qDebug()<<FN_list;
	}
	max_num ++;
	if(suffix_l<=8)
	{
		psetting.dataFileName_Suffix.sprintf("%08d", max_num);
		psetting.dataFileName_Suffix = psetting.dataFileName_Suffix.right(suffix_l);
	}
	ui->lineEdit_dataFileName_Suffix->setText(psetting.dataFileName_Suffix);
}

void paraDialog::show_detect_mode()
{
	if(psetting.elevationAngle == 0)
	{
		if(psetting.step_azAngle == 0)
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("水平单向探测"));
		else
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("水平扫描探测，每周方向数")+QString::number(360/psetting.step_azAngle));
	}
	else
		if(psetting.step_azAngle == 0)
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("单向径向探测"));
		else
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("圆锥扫描探测，每周方向数")+QString::number(360/psetting.step_azAngle));
}

void paraDialog::updates_filename()
{
	ui->lineEdit_dataFileName_ch1->setText(psetting.dataFileName_Prefix + "_ch1_" + psetting.dataFileName_Suffix + ".wld");
	ui->lineEdit_dataFileName_chA->setText(NULL);
	ui->lineEdit_dataFileName_chB->setText(NULL);
}

void paraDialog::updated_filename()
{
	ui->lineEdit_dataFileName_ch1->setText(NULL);
	ui->lineEdit_dataFileName_chA->setText(psetting.dataFileName_Prefix + "_chA_" + psetting.dataFileName_Suffix + ".wld");
	ui->lineEdit_dataFileName_chB->setText(psetting.dataFileName_Prefix + "_chB_" + psetting.dataFileName_Suffix + ".wld");
}
