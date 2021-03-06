#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ADQAPI.h"
#include "threadstore.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>

#include <QMessageBox>
#include <QFile>
#include <QDateTime>
#include <QtCore>

#include <QDesktopServices>
#include <QApplication>


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	QDir dirs;
	QString paths = dirs.currentPath()+"/"+"settings.ini";				//获取初始默认路径，并添加默认配置文件
	m_setfile.test_create_file(paths);									//检查settings.ini是否存在，若不存在则创建
	m_setfile.readFrom_file(paths);										//读取settings.ini文件
	mysetting = m_setfile.get_setting();								//mysetting获取文件中的参数

	timer1 = new QTimer(this);											//触发各个方向上开始采集
	timer2 = new QTimer(this);											//用于判断do—while
	timer2->setSingleShot(true);
	creatleftdock();													//左侧栏
	creatqwtdock();														//曲线栏
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::creatleftdock(void)									//创建左侧栏
{
	dockleft_dlg = new informationleft(this);
	dockWidget = new QDockWidget;
	dockWidget->setWidget(dockleft_dlg);
	dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

	dockleft_dlg->set_currentAngle(mysetting.start_azAngle);
	dockleft_dlg->set_groupNume(mysetting.angleNum);
	dockleft_dlg->set_groupcnt(0);

	QDateTime filepredate = QDateTime::currentDateTime();
	mysetting.dataFileName_Prefix = filepredate.toString("yyyyMMdd");	//将文件名前缀更新至最新日期

	if(mysetting.singleCh)
	{
		QString str = mysetting.dataFileName_Prefix + "_ch1_" + mysetting.dataFileName_Suffix + ".wld";
		dockleft_dlg->set_filename1(str);
		dockleft_dlg->set_filename2(NULL);
	}
	else
	{
		QString str1 = mysetting.dataFileName_Prefix + "_chA_" + mysetting.dataFileName_Suffix + ".wld";
		QString str2 = mysetting.dataFileName_Prefix + "_chB_" + mysetting.dataFileName_Suffix + ".wld";
		dockleft_dlg->set_filename1(str1);
		dockleft_dlg->set_filename2(str2);
	}
}

void MainWindow::creatqwtdock(void)										//创建曲线显示窗口
{
	if(mysetting.singleCh)												//单通道
	{
		plotWindow_1 = new PlotWindow(this);
		dockqwt_1 = new QDockWidget;

		dockqwt_1->setWidget(plotWindow_1);
		dockqwt_1->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_1);
		plotWindow_1->setMaxX(mysetting.sampleNum);						//x轴坐标值范围，初始坐标曲线设置
		plotWindow_1->set_titleUnit(1);									//通道名
		connect(dockqwt_1,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct1);
																		//双击显示全屏绘图
	}

	if(mysetting.doubleCh)												//双通道
	{
		plotWindow_1 = new PlotWindow(this);
		dockqwt_1 = new QDockWidget;
		plotWindow_2 = new PlotWindow(this);
		dockqwt_2 = new QDockWidget;

		dockqwt_1->setWidget(plotWindow_1);
		dockqwt_1->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_1);
		plotWindow_1->setMaxX(mysetting.sampleNum);
		plotWindow_1->set_titleUnit(2);
		connect(dockqwt_1,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct1);

		dockqwt_2->setWidget(plotWindow_2);
		dockqwt_2->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_2);
		plotWindow_2->setMaxX(mysetting.sampleNum);
		plotWindow_2->set_titleUnit(3);
		connect(dockqwt_2,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct2);
	}
	setPlotWindowVisible();
}

void MainWindow::setPlotWindowVisible()									//设置绘图窗口的尺寸
{
	int w = width();
	int h = height();
	w = w - 265, h = (h-25-42-22-25)/2;									//高度：菜单25，工具42，状态22，左侧边栏宽度260，再空去25的高度
	if(mysetting.singleCh)
	{
		plotWindow_1->setFixedSize(w,h);
		plotWindow_1->setMaximumSize(1920,1080);						//绘图窗口的最大尺寸
	}

	if(mysetting.doubleCh)
	{
		plotWindow_1->setFixedSize(w,h);
		plotWindow_1->setMaximumSize(1920,1080);
		plotWindow_2->setFixedSize(w,h);
		plotWindow_2->setMaximumSize(1920,1080);
	}
}

void MainWindow::resizeEvent(QResizeEvent *event)						//主窗口大小改变时，保证绘图窗口填满
{
	setPlotWindowVisible();
}

void MainWindow::dockview_ct1(bool topLevel)							//用于通道1全屏
{
	if(topLevel)
		dockqwt_1->showMaximized();
}

void MainWindow::dockview_ct2(bool topLevel)							//用于通道2全屏
{
	if(topLevel)
		dockqwt_2->showMaximized();
}

void MainWindow::on_action_searchDevice_triggered()						//action_searchDevice键
{
	conncetdevice();
}

void MainWindow::on_action_open_triggered()								//action_open键
{
	if(mysetting.DatafilePath.isEmpty())								//路径为空时，打开默认路径
	{
		QDir default_path;
		QString storepath = default_path.currentPath();
		QDesktopServices::openUrl(QUrl::fromLocalFile(storepath));
	}
	else																//路径存在时，打开对应的指定路径
	{
		path_create();
		QDesktopServices::openUrl(QUrl::fromLocalFile(mysetting.DatafilePath));
	}
}

void MainWindow::on_action_saveAs_triggered()							//action_saveAs键
{
	QString fileName = QFileDialog::getSaveFileName(this,"另存","/","data files(*.wld)");
}

void MainWindow::on_action_set_triggered()								//action_set键
{
	ParaSetDlg = new paraDialog(this);
//	ParaSetDlg->setAttribute(Qt::WA_DeleteOnClose);	//退出时自动delete，防止内存泄漏
//	ParaSetDlg->show();
//	ParaSetDlg->setWindowTitle(QString::fromLocal8Bit("设置"));
//	ParaSetDlg->setWindowIcon(QIcon(":/images/set"));

	ParaSetDlg->init_setting(mysetting);								//mysetting传递给设置窗口psetting
	ParaSetDlg->initial_para();											//参数显示在设置窗口上，并连接槽
	ParaSetDlg->on_checkBox_autocreate_datafile_clicked();				//更新文件存储路径

	if(mysetting.singleCh)												//当单通道时打开设置对话框后，创建plotwindow2窗口，点击确定以后，用于refresh中的delete
	{
		plotWindow_2 = new PlotWindow(this);
		dockqwt_2 = new QDockWidget;
	}

	if (ParaSetDlg->exec() == QDialog::Accepted)						// 确定键功能
	{
		mysetting =	ParaSetDlg->get_setting();							//mysetting获取修改后的参数

		dockleft_dlg->set_currentAngle(mysetting.start_azAngle);		//更新左侧栏
		dockleft_dlg->set_groupNume(mysetting.angleNum);
		dockleft_dlg->set_groupcnt(0);
		if(mysetting.singleCh)
		{
			QString str = mysetting.dataFileName_Prefix + "_ch1_" + mysetting.dataFileName_Suffix + ".wld";
			dockleft_dlg->set_filename1(str);
			dockleft_dlg->set_filename2(NULL);
			refresh();													//更新绘图窗口
		}
		else
		{
			QString str1 = mysetting.dataFileName_Prefix + "_chA_" + mysetting.dataFileName_Suffix + ".wld";
			QString str2 = mysetting.dataFileName_Prefix + "_chB_" + mysetting.dataFileName_Suffix + ".wld";
			dockleft_dlg->set_filename1(str1);
			dockleft_dlg->set_filename2(str2);
			refresh();
		}
		delete ParaSetDlg;												//防止内存泄漏
	}
	else
		if(mysetting.singleCh)											//点击非确定键，则删除创建的plotwindow2窗口
		{
			delete plotWindow_2;
			delete dockqwt_2;
		}
}

void MainWindow::path_create()											//数据存储文件夹的创建
{
	QString currentpath = mysetting.DatafilePath;
	QDir mypath;
	if(!mypath.exists(currentpath))										//文件夹不存在，创建文件夹
		mypath.mkpath(currentpath);
}

void MainWindow::refresh()												//paradialog重新设置后，对绘图曲线部分进行更新
{
	delete plotWindow_1;
	delete dockqwt_1;
	delete plotWindow_2;
	delete dockqwt_2;
	creatqwtdock();
}

void MainWindow::on_action_start_triggered()							//采集菜单中的开始键
{
	path_create();														//数据存储文件夹的创建
	conncetdevice();													//连接设备
	stopped = false;													//stopped为false。能够采集

	clock_source = 0;													//时钟源选择0，内部时钟，内部参考
	n_sample_skip = 1;													//采样间隔设为1，表示无采样间隔
	qDebug() << "n_sample_skip = " << n_sample_skip;
	if(ADQ212_SetSampleSkip(adq_cu,1,n_sample_skip) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleSkip"));
		return;
	}	

	strs = mysetting.dataFileName_Suffix;								//文件名可自动加1
	strsuffix =strs.toInt();
	numbercollect = 0;

	connect(timer2,SIGNAL(timeout()),this,SLOT(collect_over()));		//建立用于检查do-while的定时器
	if(mysetting.singleCh)												//单通道采集
	{			
		connect(timer1,SIGNAL(timeout()),this,SLOT(singlecollect()));	//建立定时器并连接函数
		singleset();
		singlecollect();
		timer1->start(10000);											//10s一次
	}
	else																//双通道采集
	{
		connect(timer1,SIGNAL(timeout()),this,SLOT(doublecollect()));
		doubleset();
		doublecollect();
		timer1->start(10000);
	}
}

void MainWindow::on_action_stop_triggered()
{
	stopped = true;
}

void MainWindow::singleset()											//单通道参数设置
{
	trig_mode = 3;
	if(ADQ212_SetTriggerMode(adq_cu,1,trig_mode) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigMode"));
		return;
	}
	trig_level = mysetting.triggleLevel;
	if(ADQ212_SetLvlTrigLevel(adq_cu,1,trig_level) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigLevel"));
		return;
	}
	trig_flank = 1;
	if(ADQ212_SetLvlTrigFlank(adq_cu,1,trig_flank) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigFlank"));
		return;
	}
	trig_channel = 1;
	if(ADQ212_SetLvlTrigChannel(adq_cu,1,trig_channel) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigChannel"));
		return;
	}

	qDebug() << "trig_mode = " << trig_mode;							//触发模式
	qDebug() << "clock_source = " << clock_source;
	if(ADQ212_SetClockSource(adq_cu,1,clock_source) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("ClockSource"));
		return;
	}

	int f = mysetting.sampleFreq;
	switch (f) {
	case 550:
		pll_divider = 2;
		break;
	case 367:
		pll_divider = 3;
		break;
	case 275:
		pll_divider = 4;
		break;
	case 220:
		pll_divider = 5;
		break;
	case 183:
		pll_divider = 6;
		break;
	case 157:
		pll_divider = 7;
		break;
	case 138:
		pll_divider = 8;
		break;
	case 122:
		pll_divider = 9;
		break;
	case 110:
		pll_divider = 10;
		break;
	case 100:
		pll_divider = 11;
		break;
	default:
		break;
	}
	if(ADQ212_SetPllFreqDivider(adq_cu,1,pll_divider) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleFreq"));
		return;
	}

	number_of_records = mysetting.plsAccNum;										//脉冲数
	samples_per_record = mysetting.sampleNum;										//采样点数

	if(ADQ212_MultiRecordSetup(adq_cu,1,number_of_records,samples_per_record) == 0)	//设置内存缓冲区
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleNum"));
		return;
	}
}

void MainWindow::singlecollect()													//单通道采集和存储
{
	direction_angle = mysetting.start_azAngle+numbercollect*mysetting.step_azAngle;	//更新左侧栏
	if(direction_angle > 360)
		direction_angle = direction_angle%360;

	dockleft_dlg->set_currentAngle(direction_angle);
	dockleft_dlg->set_groupcnt(numbercollect+1);
	strs = QString::number(strsuffix,10);											//更新采集文件的后缀序号

	QString str = mysetting.dataFileName_Prefix + "_ch1_" + strs + ".wld";
	dockleft_dlg->set_filename1(str);
	dockleft_dlg->set_filename2(NULL);												//更新文件名

	if(ADQ212_DisarmTrigger(adq_cu,1) == 0)											//解除触发，内存计数重置Disarm unit
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed1"));
		return;
	}

	QDateTime collectTime = QDateTime::currentDateTime();							//采集开始时间
	timestr = collectTime.toString("yyyy/MM/dd hh:mm:ss");

	if(ADQ212_ArmTrigger(adq_cu,1) == 0)											//Arm unit
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed2"));
		return;
	}

	qDebug() << "Please trig your device to collect data.";
	int trigged;
	timer2->start(4000);
	do
	{
		trigged = ADQ212_GetTriggedAll(adq_cu,1);									//Trigger unit
		if(timer2->remainingTime() == 0)
			return;
	}while(trigged == 0);
	timer2->stop();

	qDebug() << "Device trigged.";
	int *data_1_ptr_addr0 = ADQ212_GetPtrDataChA(adq_cu,1);
	qDebug() << "Collecting data,plesase wait...";

	qint16 *rd_data1 = new qint16[samples_per_record*number_of_records];
	for(unsigned int i = 0; i < number_of_records; i++)								//写入采集数据
	{
		int rn1 = 0;
		unsigned int samples_to_collect = samples_per_record;
		while(samples_to_collect > 0)
		{
			int collect_result = ADQ212_CollectRecord(adq_cu,1,i);
			unsigned int samples_in_buffer = qMin(ADQ212_GetSamplesPerPage(adq_cu,1),samples_to_collect);

			if(collect_result)
			{
				for(unsigned int j = 0; j < samples_in_buffer; j++)
				{
					rd_data1[i*samples_per_record + rn1] = *(data_1_ptr_addr0 + j);
					rn1++;
				}
				samples_to_collect -= samples_in_buffer;
			}
			else
			{
				qDebug() << "Collect next data page failed!";
				samples_to_collect = 0;
				i = number_of_records;
			}
		}
	}
	if(!threadA.isRunning())
	{
		threadA.fileDataPara(mysetting);								//mysetting值传递给threadstore
		threadA.otherpara(strs,timestr,direction_angle);				//组数，时间，方位角传递给threadstore
		threadA.s_memcpy(rd_data1);										//采样数据传递给threadstore
		threadA.start();												//启动threadstore线程
	}
	else
		if(!threadB.isRunning())
		{
			threadB.fileDataPara(mysetting);
			threadB.otherpara(strs,timestr,direction_angle);
			threadB.s_memcpy(rd_data1);
			threadB.start();
		}
		else
			if(!threadC.isRunning())
			{
				threadC.fileDataPara(mysetting);
				threadC.otherpara(strs,timestr,direction_angle);
				threadC.s_memcpy(rd_data1);
				threadC.start();
			}
			else
			{
				threadD.fileDataPara(mysetting);
				threadD.otherpara(strs,timestr,direction_angle);
				threadD.s_memcpy(rd_data1);
				threadD.start();
			}

	plotWindow_1->datashow(rd_data1,mysetting.sampleNum,mysetting.plsAccNum);		//绘图窗口显示最后一组脉冲
	delete[] rd_data1;

	strsuffix++;
	numbercollect++;																//下一组采集组数
	if((numbercollect >= mysetting.angleNum)||(stopped == true))					//判断是否完成设置组数
		collect_over();
}

void MainWindow::collect_over()
{
	timer1->stop();
	ADQ212_DisarmTrigger(adq_cu,1);
	ADQ212_MultiRecordClose(adq_cu,1);

	ADQ212_GetTrigPoint(adq_cu,1);
	ADQ212_GetTriggedCh(adq_cu,1);
	int overflow = ADQ212_GetOverflow(adq_cu,1);
	if(overflow == 1)
		qDebug() << "Sample overflow in batch.";
	qDebug() << "Collect finished.";
	QMessageBox::information(this,QString::fromLocal8Bit("Information"),QString::fromLocal8Bit("Collect finished."));		//设置界面提示窗口，提示采集完成
	DeleteADQControlUnit(adq_cu);
}

void MainWindow::doubleset()														//双通道采集
{
	trig_mode = 2;
	if(ADQ212_SetTriggerMode(adq_cu,1,trig_mode) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigMode"));
		return;
	}

	qDebug() << "trig_mode = " << trig_mode;										//触发模式
	qDebug() << "clock_source = " << clock_source;
	if(ADQ212_SetClockSource(adq_cu,1,clock_source) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("ClockSource"));
		return;
	}
	int f = mysetting.sampleFreq;
	switch (f) {
	case 550:
		pll_divider = 2;
		break;
	case 367:
		pll_divider = 3;
		break;
	case 275:
		pll_divider = 4;
		break;
	case 220:
		pll_divider = 5;
		break;
	case 183:
		pll_divider = 6;
		break;
	case 157:
		pll_divider = 7;
		break;
	case 138:
		pll_divider = 8;
		break;
	case 122:
		pll_divider = 9;
		break;
	case 110:
		pll_divider = 10;
		break;
	case 100:
		pll_divider = 11;
		break;
	default:
		break;
	}
	if(ADQ212_SetPllFreqDivider(adq_cu,1,pll_divider) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Samplefreq"));
		return;
	}

	number_of_records = mysetting.plsAccNum;
	samples_per_record = mysetting.sampleNum;
	if(ADQ212_MultiRecordSetup(adq_cu,1,number_of_records,samples_per_record) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleNum"));
		return;
	}
}

void MainWindow::doublecollect()													//双通道采集和存储
{
	direction_angle = mysetting.start_azAngle + numbercollect*mysetting.step_azAngle;
	if(direction_angle > 360)
		direction_angle = direction_angle%360;
	dockleft_dlg->set_currentAngle(direction_angle);
	dockleft_dlg->set_groupcnt(numbercollect+1);
	strs = QString::number(strsuffix,10);

	QString strA = mysetting.dataFileName_Prefix + "_chA_" + strs + ".wld";
	QString strB = mysetting.dataFileName_Prefix + "_chB_" + strs + ".wld";
	dockleft_dlg->set_filename1(strA);
	dockleft_dlg->set_filename2(strB);

	if(ADQ212_DisarmTrigger(adq_cu,1) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed"));
		return;
	}

	QDateTime collectTime = QDateTime::currentDateTime();
	timestr = collectTime.toString("yyyy/MM/dd hh:mm:ss");

	if(ADQ212_ArmTrigger(adq_cu,1) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed"));
		return;
	}

	qDebug() << "Please trig your device to collect data.";
	int trigged;
	timer2->start(4000);
	do
	{
		trigged = ADQ212_GetTriggedAll(adq_cu,1);
		if (timer2->remainingTime() == 0)
			return;
	}while(trigged == 0);
	timer2->stop();

	qDebug() << "Device trigged.";

	int *data_a_ptr_addr0 = ADQ212_GetPtrDataChA(adq_cu,1);
	int *data_b_ptr_addr0 = ADQ212_GetPtrDataChB(adq_cu,1);

	qDebug() << "Collecting data,plesase wait...";

	qint16 *rd_dataa = new qint16[samples_per_record*number_of_records];
	qint16 *rd_datab = new qint16[samples_per_record*number_of_records];
	for(unsigned int i = 0; i < number_of_records; i++)
	{
		int rna = 0;
		int rnb = 0;
		unsigned int samples_to_collect = samples_per_record;
		while(samples_to_collect > 0)
		{
			int collect_result = ADQ212_CollectRecord(adq_cu,1,i);
			unsigned int samples_in_buffer = qMin(ADQ212_GetSamplesPerPage(adq_cu,1),samples_to_collect);

			if(collect_result)
			{
				for(unsigned int j = 0; j < samples_in_buffer; j++)
				{
					rd_dataa[i*samples_per_record + rna] = *(data_a_ptr_addr0 + j);
					rd_datab[i*samples_per_record + rnb] = *(data_b_ptr_addr0 + j);
					rna++;
					rnb++;
				}
				samples_to_collect -= samples_in_buffer;
			}
			else
			{
				qDebug() << "Collect next data page failed!";
				samples_to_collect = 0;
				i = number_of_records;
			}
		}
	}

	if(!threadA.isRunning())
	{
		threadA.fileDataPara(mysetting);								//mysetting值传递给threadstore
		threadA.otherpara(strs,timestr,direction_angle);				//组数，时间，方位角传递给threadstore
		threadA.d_memcpy(rd_dataa,rd_datab);							//采样数据传递给threadstore
		threadA.start();												//启动threadstore线程
	}
	else
		if(!threadB.isRunning())
		{
			threadB.fileDataPara(mysetting);
			threadB.otherpara(strs,timestr,direction_angle);
			threadB.d_memcpy(rd_dataa,rd_datab);
			threadB.start();
		}
		else
			if(!threadC.isRunning())
			{
				threadC.fileDataPara(mysetting);
				threadC.otherpara(strs,timestr,direction_angle);
				threadC.d_memcpy(rd_dataa,rd_datab);
				threadC.start();
			}
			else
			{
				threadD.fileDataPara(mysetting);
				threadD.otherpara(strs,timestr,direction_angle);
				threadD.d_memcpy(rd_dataa,rd_datab);
				threadD.start();
			}
	plotWindow_1->datashow(rd_dataa,mysetting.sampleNum,mysetting.plsAccNum);	//绘图窗口显示a最后一组脉冲
	delete[] rd_dataa;
	plotWindow_2->datashow(rd_datab,mysetting.sampleNum,mysetting.plsAccNum);	//绘图窗口显示b最后一组脉冲
	delete[] rd_datab;

	strsuffix++;
	numbercollect++;

	if((numbercollect >= mysetting.angleNum)||(stopped == true))				//判断是否完成设置组数
		collect_over();
}

void MainWindow::conncetdevice()												//查找连接ADQ212设备
{
	adq_cu = CreateADQControlUnit();											//用于查找和建立与ADQ设备之间的链接
	qDebug() << "adq_cu = " << adq_cu;
	int n_of_devices = 0;
	int n_of_failed = 0;
	n_of_devices = ADQControlUnit_FindDevices(adq_cu);							//找到所有与电脑连接的ADQ，并创建一个指针列表，返回找到设备的总数
	int n_of_ADQ212 = ADQControlUnit_NofADQ212(adq_cu);							//返回找到ADQ212设备的数量
	n_of_failed = ADQControlUnit_GetFailedDeviceCount(adq_cu);					//返回找到的单元数量

	if(n_of_failed > 0)
		QMessageBox::warning(this,QString::fromLocal8Bit("Warning"),QString::fromLocal8Bit("设备启动失败"));
	if(n_of_devices == 0)
		QMessageBox::information(this,QString::fromLocal8Bit("Warning"),QString::fromLocal8Bit("No ADQ devices found"));

	if(n_of_ADQ212 != 0)
	{
		QMessageBox::information(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("ADQ212 found"));
		char *BSN = ADQ212_GetBoardSerialNumber(adq_cu,1);						//设备序列号
		int *Revision = ADQ212_GetRevision(adq_cu,1);							//返回设备的revision
		qDebug() << BSN;
		qDebug() << Revision[0];
		qDebug() << Revision[1];
		qDebug() << Revision[2];												//0-2是FPGA#2(comm)的信息，3-5是FPGA#1(alg)的信息
		qDebug() << Revision[3];												//revision数
		qDebug() << Revision[4];												//0表示SVN Managed，1表示Local Copy
		qDebug() << Revision[5];												//0表示SVN Updated，1表示Mixed Revision

		qDebug() << "set and  than click collect start";
	}
}

