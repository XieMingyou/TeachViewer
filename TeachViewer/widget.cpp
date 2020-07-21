#pragma execution_character_set("utf-8")
#include "widget.h"
#include "ui_widget.h"
#include"tinystr.h"
#include"tinyxml.h"
#include"mydelegate.h"
#include<iostream>
#include<QPixmap>
#include<QTime>
#include<QPalette>
# include<QMenu>
#include<QAction>
#include<QDebug>
#include<QDir>
#include<QLineEdit>
#include<QInputDialog>
#include<QMessageBox>
#include<QTextCodec>
#include<QTextLayout>
#include<QTextBlock>
#include<QItemSelectionModel>
#include<QModelIndex>
#include<QTreeView>
#include<QMouseEvent>
#include<QItemDelegate>
#include<QDialog>
#include<QFormLayout>
#include<QDialogButtonBox>
#include<QTextCursor>
#include<vector>
#include<string>
#include<fstream>
#include<QtXml>
#include "transferThread.h"
#include "updateThread.h"
#include "dataDeclaration.h"

using namespace std;

//示教器发送给控制器的数据
int emergeStop; //机器人是否急停，0：未急停，1：急停
int enableState; //机器人使能状态，0：未使能，1：使能
int operateMode; //机器人操作模式，0：手动，1：自动
int activeState; //程序的激活状态，0：未激活，1：激活
int runState; //程序的运行状态，0：暂停，1：运行中
int step; //程序的执行方式，0：连续，1：单步
int jog; //手动示教时所选坐标系，0：轴关节坐标系，1：世界坐标系，2：工具手坐标系
int coordinate; //手动示教时选中的坐标，0：未示教，1-6：第x个坐标
int upOrDown; //手动示教时对选中坐标的增大或减小，0：减小，1：增大
int ovr; //相对最大运动参数的百分比
int progLine; //程序执行到的行数
int transferZip; //是否传输文件压缩包，0：不传输，1：传输
int zipSize; //压缩包大小
string projName; //主程序所在项目的名称
string varProgName; //主程序名称

//控制器发送给示教器的数据
struct AXISPOS axisPos; //机器人在轴关节坐标系下的位置坐标
struct CARTPOS cartPos; //机器人在世界坐标系下的位置坐标
struct DRIVEPOS drivePos; //机器人伺服电机的位置坐标
int activeProg; //是否激活程序，0：否，1：是
int runProg; //是否运行程序，0：否，1：是
int hasReachSingularity; //是否到达机器人的奇异点，0：否，1：是。用于控制示教时，机器人在直角坐标系下到达奇异点后将不再移动
vector<pair<int, string>> infoReport; //信息报告的内容

double pulseEquivalent[6]; //每个轴的脉冲当量
double averagePulseEquivalent; //六轴的平均脉冲当量
struct DYNAMIC curDynamic; //当前的机器人运动参数
struct CARTSYS curTool; //当前的工具坐标系
unsigned long contiRunState; //机器人插补运动状态

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //设置菜单栏上显示学校的图标
    QPixmap school_pic(":/image/school.png");
    school_pic=school_pic.scaled(ui->label_school->width(),ui->label_school->height());//图片自适应label变化
    ui->label_school->setPixmap(school_pic);//将图片添加到label中
    //设置菜单栏显示时间的label
    timerEvent(0);
    startTimer(1000);
    //布局变量管理模块的基本界面
    set_variavle_charge_widget();
    //布局项目管理模块的基本界面
    set_project_charge_widget();
    //布局程序管理模块的部分基本界面（往变量按钮添加菜单选项等）
    set_instruction_set();
    //布局位置管理模块的基本界面
    set_coordinate_charge_widget();
    //布局信息报告管理模块的基本界面
    set_information_charge_widget();
    //创建数据传输线程，与控制器端进行交互
    TransferThread *transferThread = new TransferThread; //创建数据传输线程对象
    transferThread->start(); //启动数据传输线程
    //创建数据更新线程，对Ui界面上的数据进行更新
    UpdateThread *updateThread = new UpdateThread; //创建数据传输线程对象
    updateThread->tablemodel_coordinate = tablemodel_coordinate; //将位置管理模块的模型指针传入数据更新线程
    on_btn_coordinate_world_clicked(); //默认显示世界坐标系
    updateThread->coordinateShow = &coordinateShow; //将位置管理模块中选中要显示的坐标系标志传入数据更新线程
    updateThread->start(); //启动数据传输线程
}
Widget::~Widget()
{
    delete ui;
}
//时间事件处理函数的实现
void Widget::timerEvent(QTimerEvent *event){
    Q_UNUSED(event);
    ui->label_time->setText(QTime::currentTime().toString("hh:mm:ss"));
}
//菜单栏read按钮的槽函数
void Widget::on_btn_read_information_clicked()
{
    if(!label_information_text.isEmpty()){
        if(!label_information_text.isEmpty()){
            QString temp=label_information_text.pop();
            ui->label_information->setText(temp);
        }
        else{
            ui->label_information->setText("历史信息已清空");
        }
    }
    else{
        ui->label_information->setText("历史信息已清空");
    }
}
//七个切换模块界面控件的槽函数
//用户自定义界面的切换
void Widget::on_btn_user_diy_clicked()
{
    int indexpage=0;
    switch_page(indexpage);
    cout<<"user_diy_widget"<<endl;
}
//配置管理界面的切换
void Widget::on_btn_config_charge_clicked()
{
    int indexpage=1;
    switch_page(indexpage);
    cout<<"config_charge_widget"<<endl;
}
//变量管理界面的切换
void Widget::on_btn_variable_charge_clicked()
{
    int indexpage=2;
    switch_page(indexpage);
    cout<<"variable_charge_widget"<<endl;
    //解析全局变量作用域
    set_all_range_variable_xml_update();
    //解析程序项目变量作用域
    set_project_range_variable_xml_update();
    //解析程序变量作用域
    set_program_range_variable_xml_update();
    //设置变量类型下拉框显示为全部
    ui->comboBox_variable_type->setCurrentIndex(0);

}
//布局变量管理模块的基本界面
void Widget::set_variavle_charge_widget(){
    //为变量按钮添加action
    QMenu* variable_menu=new QMenu(ui->page_variable);
    ui->btn_variable_do->setMenu(variable_menu);
    //初始化变量按钮下的两个控件
    variable_new=new QAction("新建",ui->page_variable);
    variable_delete=new QAction("删除",ui->page_variable);
    variable_copy=new QAction("复制",ui->page_variable);
    variable_paste=new QAction("粘贴",ui->page_variable);
    variable_cut=new QAction("剪切",ui->page_variable);
    variable_rename=new QAction("重命名",ui->page_variable);
    //将控件添加进menu中
    variable_menu->addAction(variable_new);
    variable_menu->addAction(variable_delete);
    variable_menu->addAction(variable_copy);
    variable_menu->addAction(variable_paste);
    variable_menu->addAction(variable_cut);
    variable_menu->addAction(variable_rename);

    //添加控件的信号槽
    connect(variable_new,SIGNAL(triggered()),this,SLOT(slt_variable_new_variable()));
    connect(variable_delete,SIGNAL(triggered()),this,SLOT(slt_variable_delete_variable()));
    connect(variable_copy,SIGNAL(triggered()),this,SLOT(slt_variable_copy_variable()));
    connect(variable_paste,SIGNAL(triggered()),this,SLOT(slt_variable_paste_variable()));
    connect(variable_cut,SIGNAL(triggered()),this,SLOT(slt_variable_cut_variable()));
    connect(variable_rename,SIGNAL(triggered()),this,SLOT(slt_variable_rename_variable()));
    //开始创建视图
    treemodel_variable=new QStandardItemModel(0,2,ui->page_variable);
    treemodel_variable->setHeaderData(0,Qt::Horizontal,"变量");
    treemodel_variable->setHeaderData(1,Qt::Horizontal,"数值");
    root_variable=treemodel_variable->invisibleRootItem();//根节点，不可见
    ui->treeView_variable->setModel(treemodel_variable);
    ui->treeView_variable->setColumnWidth(0,300);
    ui->treeView_variable->setColumnWidth(1,200);
    //添加全局的节点
    QStandardItem* item_all=new QStandardItem("全局");
    root_variable->appendRow(item_all);
    //当选中treeview_variable时，控制项目管理模块按钮是否可以点击
    connect(ui->treeView_variable->selectionModel(),&QItemSelectionModel::currentChanged,this,&Widget::slot_treeview_variable_CurrentChanged);
    //设置Qcombox的信号槽
    connect(ui->comboBox_variable_type,SIGNAL(currentIndexChanged(int )),this,SLOT(slt_deal_variable_type(int)));
}
//处理下拉框不同选项的槽函数
void Widget::slt_deal_variable_type(int a){
    if(a==0){
        //全部变量类型
        //解析全局变量作用域
        set_all_range_variable_xml_update();
        //解析程序项目变量作用域
        set_project_range_variable_xml_update();
        //解析程序变量作用域
        set_program_range_variable_xml_update();
    }
    if(a==1){
        //INTEGER
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("INTEGER");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("INTEGER");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("INTEGER");
    }
    if(a==2){
        //FLOAT
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("FLOAT");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("FLOAT");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("FLOAT");
    }
    if(a==3){
        //BOOL
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("BOOL");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("BOOL");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("BOOL");
    }
    if(a==4){
        //STRING
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("STRING");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("STRING");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("STRING");
    }
    if(a==5){
        //DYNAMIC
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("DYNAMIC");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("DYNAMIC");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("DYNAMIC");
    }
    if(a==6){
        //AXISPOS
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("AXISPOS");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("AXISPOS");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("AXISPOS");
    }
    if(a==7){
        //Base
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("TOOL");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("TOOL");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("TOOL");
    }
    if(a==8){
        //CARTREFSYS
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("CARTREFSYS");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("CARTREFSYS");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("CARTREFSYS");
    }
    if(a==9){
        //CARTPOS
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("CARTPOS");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("CARTPOS");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("CARTPOS");
    }
    if(a==10){
        //OVERLAP
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("OVERLAP");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("OVERLAP");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("OVERLAP");
    }
}
//解析全局变量作用域中特定的数据类型，将解析的信息呈现在treeview_variable中
void Widget::set_detetmine_type_all_range_variable_xml_update(QString type){
    //每次执行该函数时，清空模型，便于刷新
    treemodel_variable->clear();
    treemodel_variable=new QStandardItemModel(0,2,ui->page_variable);
    treemodel_variable->setHeaderData(0,Qt::Horizontal,"变量");
    treemodel_variable->setHeaderData(1,Qt::Horizontal,"数值");
    root_variable=treemodel_variable->invisibleRootItem();//根节点，不可见
    ui->treeView_variable->setModel(treemodel_variable);
    ui->treeView_variable->setColumnWidth(0,300);
    ui->treeView_variable->setColumnWidth(1,200);
    //添加全局的节点
    QStandardItem* item_all=new QStandardItem("全局");
    root_variable->appendRow(item_all);

    //创建一个xml文档
    TiXmlDocument* doc=new TiXmlDocument;
    QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
    QStandardItem* all_item=root_variable->child(0,0);
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"load file success";
    }
    else{
        qDebug()<<"load file failed";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"no root";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            //需要判断当前xml节点的变量名，all_item下是否存在
            int judge=0;
            for(int i=0;i<all_item->rowCount();i++){
                QString now_all_name=all_item->child(i,0)->text();//当前每个节点的名称

                if(node_name->Value()==now_all_name){
                    judge++;//重名的话，递增
                    break;
                }
            }
            //如果judge还为0，证明想加入的节点名称和当前all_item没有重名
            if(judge==0){
                if(node_type->Value()==type){
                    QString name=QString::fromStdString(node_name->Value());
                    QString insert_name=all_item->text()+name;
                    type_attribute_variable.insert(insert_name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
                    type_range_variable.insert(insert_name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
                    //将变量名插入到当前模型当中
                    QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                    //将变量名添加进节点中
                    all_item->appendRow(item_name);
                    //然后将变量下的对应参数插入对应的变量节点中
                    int count=0;//参入参数时候的标志
                    for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                        //获取每个参数节点的值
                        string value=node_value->GetText();
                        QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                        //获取每个参数节点的名称
                        TiXmlAttribute* value_name=node_value->FirstAttribute();
                        string name_value=value_name->Value();
                        QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                        //先插入参数节点的名称
                        item_name->appendRow(item_value_name);//插入名称到变量节点中
                        //接下来将参数值的item插入到对应模型的item位置中
                        item_name->setChild(count,1,item_value);
                        count++;
                    }
                }
            }
        }
    }
    //使得节点的展开深度为1
    ui->treeView_variable->expandToDepth(0);
}
//解析项目变量作用域中特定的数据类型，将解析的信息呈现在treeview_variable中
void Widget::set_determine_type_project_range_variable_xml_update(QString type){
    //遍历所有的项目变量xml文件
    QDir dir(root_xml_file);
    dir.setFilter(QDir::Dirs);//过滤配置，接受文件
    QFileInfoList list=dir.entryInfoList();
    QStringList filelist;
    for(int i=0;i<list.count();i++){
        QString filename=list.at(i).fileName();
        if(filename=="."||filename==".."){

        }
        else{
            QString name=filename;//获取非"."和".."的文件
            //判断当前treemodel下是否有和当前加入的项目名称同名的
            int judge_project=0;
            for(int j=0;j<root_variable->rowCount();j++){
                if(root_variable->child(j,0)){
                    QString judge_name=root_variable->child(j,0)->text();
                    QString temp_name="项目"+name;
                    if(judge_name==temp_name){
                        judge_project++;
                        break;
                    }
                }
            }
            //若为0，证明当前想插入的节点可以插入，没有重名的
            if(judge_project==0){
                QString project_name="项目"+name;
                QStandardItem* project_item=new QStandardItem(project_name);
                root_variable->appendRow(project_item);
                //插入项目节点之后，开始遍历项目文件下的项目变量
                QString path1=root_xml_file+"//"+name+"//"+name+".xml";
                //创建一个xml文档
                TiXmlDocument* doc=new TiXmlDocument;
                if(doc->LoadFile(path1.toStdString().c_str())){
                    qDebug()<<"load file success";
                }
                else{
                    qDebug()<<"load file failed";
                }
                //首先获取根节点，看根节点是否存在
                TiXmlElement* root_robot=doc->RootElement();
                if(root_robot==NULL){
                    qDebug()<<"no root";
                }
                else{
                    //开始解析节点下的子节点
                    for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                        TiXmlAttribute* node_name=node->FirstAttribute();//节点变量名
                        TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                        TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                        if(node_type->Value()==type){
                            QString name=QString::fromStdString(node_name->Value());
                            QString insert_name=project_item->text()+name;
                            type_attribute_variable.insert(insert_name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
                            type_range_variable.insert(insert_name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
                            //将变量名插入到当前模型当中
                            QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                            //将变量名添加进节点中
                            project_item->appendRow(item_name);
                            //然后将变量下的对应参数插入对应的变量节点中
                            int count=0;//参入参数时候的标志
                            for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                                //获取每个参数节点的值
                                string value=node_value->GetText();
                                QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                                //获取每个参数节点的名称
                                TiXmlAttribute* value_name=node_value->FirstAttribute();
                                string name_value=value_name->Value();
                                QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                                //先插入参数节点的名称
                                item_name->appendRow(item_value_name);//插入名称到变量节点中
                                //接下来将参数值的item插入到对应模型的item位置中
                                item_name->setChild(count,1,item_value);
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }
    ui->treeView_variable->expandToDepth(0);
}
//解析程序变量作用域中特定的数据类型，将解析的信息呈现在treeview_variable中
void Widget::set_detemine_type_program_range_variable_xml_update(QString type){
    //遍历所有的项目变量xml文件
    QDir dir(root_xml_file);
    dir.setFilter(QDir::Dirs);//过滤配置，接受文件
    QFileInfoList list=dir.entryInfoList();
    QStringList filelist;
    for(int i=0;i<list.count();i++){
        QString filename=list.at(i).fileName();
        if(filename=="."||filename==".."){

        }
        else{
            QString name=filename;//获取非"."和".."的文件
            //解析项目下的各xml文件
            QString project_path=root_xml_file+"//"+name;
            QDir dir1(project_path);
            dir1.setFilter(QDir::Files);
            QFileInfoList list1=dir1.entryInfoList();
            QStringList fileList_file;
            for(int j=0;j<list1.count();j++){
                QString file_name=list1.at(j).fileName();
                //筛选不和项目重名的xml文件进行解析
                QString judge_project_name=name+".xml";
                if(judge_project_name!=file_name){
                    QString program_file=file_name;//程序变量xml文件
                    //插入程序节点之前，判断treeview中是否已经有重名的
                    int judge=0;
                    for(int k=0;k<root_variable->rowCount();k++){
                        if(root_variable->child(k,0)){
                            QString judge_name=root_variable->child(k,0)->text();
                            QString temp_name="程序"+name;
                            if(judge_name==temp_name){
                                judge++;
                                break;
                            }
                        }
                    }
                    //若为0，证明当前想插入的节点可以插入，没有重名的
                    if(judge==0){
                        QString program_name="程序"+program_file.left(program_file.length()-4)+"["+name+"]";
                        QStandardItem* program_item=new QStandardItem(program_name);
                        root_variable->appendRow(program_item);
                        //插入程序节点之后，开始遍历程序文件下的程序变量
                        QString program_path=root_xml_file+"//"+name+"//"+program_file;
                        //创建一个xml文档
                        TiXmlDocument* doc=new TiXmlDocument;
                        if(doc->LoadFile(program_path.toStdString().c_str())){
                            qDebug()<<program_path+" load file success";
                        }
                        else{
                            qDebug()<<"load file failed";
                        }
                        //首先获取根节点，看是否存在
                        TiXmlElement* root_robot=doc->RootElement();
                        if(root_robot==NULL){
                            qDebug()<<"no root";
                        }
                        else{
                            //开始解析节点下的子节点
                            for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                                TiXmlAttribute* node_name=node->FirstAttribute();//节点变量名
                                TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                                TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                                if(node_type->Value()==type){
                                    QString name=QString::fromStdString(node_name->Value());
                                    QString insert_name=program_item->text()+name;
                                    type_attribute_variable.insert(insert_name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
                                    type_range_variable.insert(insert_name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
                                    //将变量名插入到当前模型当中
                                    QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                                    //将变量名添加进节点中
                                    program_item->appendRow(item_name);
                                    //然后将变量下的对应参数插入对应的变量节点中
                                    int count=0;//参入参数时候的标志
                                    for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                                        //获取每个参数节点的值
                                        string value=node_value->GetText();
                                        QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                                        //获取每个参数节点的名称
                                        TiXmlAttribute* value_name=node_value->FirstAttribute();
                                        string name_value=value_name->Value();
                                        QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                                        //先插入参数节点的名称
                                        item_name->appendRow(item_value_name);//插入名称到变量节点中
                                        //接下来将参数值的item插入到对应模型的item位置中
                                        item_name->setChild(count,1,item_value);
                                        count++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ui->treeView_variable->expandToDepth(0);
}
//选中treeview_variable上的选项时，触发该槽函数
void Widget::slot_treeview_variable_CurrentChanged(const QModelIndex& t, const QModelIndex& previous){
    QStandardItem* item1=treemodel_variable->itemFromIndex(t);
    QStandardItem* item2=treemodel_variable->itemFromIndex(previous);
    qDebug()<<"选中的treeview_variable上的选项";

}
//切换到变量管理界面时，该界面解析程序变量xml文件，将解析的信息呈现在treeview_variable上
void Widget::set_program_range_variable_xml_update(){
    //遍历所有的项目变量xml文件
    QDir dir(root_xml_file);
    dir.setFilter(QDir::Dirs);//过滤配置，接受文件
    QFileInfoList list=dir.entryInfoList();
    QStringList filelist;
    for(int i=0;i<list.count();i++){
        QString filename=list.at(i).fileName();
        if(filename=="."||filename==".."){

        }
        else{
            QString name=filename;//获取非"."和".."的文件
            //解析项目下的各xml文件
            QString project_path=root_xml_file+"//"+name;
            QDir dir1(project_path);
            dir1.setFilter(QDir::Files);
            QFileInfoList list1=dir1.entryInfoList();
            QStringList fileList_file;
            for(int j=0;j<list1.count();j++){
                QString file_name=list1.at(j).fileName();
                //筛选不和项目重名的xml文件进行解析
                QString judge_project_name=name+".xml";
                if(judge_project_name!=file_name){
                    QString program_file=file_name;//程序变量xml文件
                    //插入程序节点之前，判断treeview中是否已经有重名的
                    int judge=0;
                    for(int k=0;k<root_variable->rowCount();k++){
                        if(root_variable->child(k,0)){
                            QString judge_name=root_variable->child(k,0)->text();
                            QString temp_name="程序"+name;
                            if(judge_name==temp_name){
                                judge++;
                                break;
                            }
                        }
                    }
                    //若为0，证明当前想插入的节点可以插入，没有重名的
                    if(judge==0){
                        QString program_name="程序"+program_file.left(program_file.length()-4)+"["+name+"]";
                        QStandardItem* program_item=new QStandardItem(program_name);
                        root_variable->appendRow(program_item);
                        //插入程序节点之后，开始遍历程序文件下的程序变量
                        QString program_path=root_xml_file+"//"+name+"//"+program_file;
                        //创建一个xml文档
                        TiXmlDocument* doc=new TiXmlDocument;
                        if(doc->LoadFile(program_path.toStdString().c_str())){
                            qDebug()<<program_path+" load file success";
                        }
                        else{
                            qDebug()<<"load file failed";
                        }
                        //首先获取根节点，看是否存在
                        TiXmlElement* root_robot=doc->RootElement();
                        if(root_robot==NULL){
                            qDebug()<<"no root";
                        }
                        else{
                            //开始解析节点下的子节点
                            for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                                TiXmlAttribute* node_name=node->FirstAttribute();//节点变量名
                                TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                                TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                                QString name=QString::fromStdString(node_name->Value());
                                QString insert_name=program_item->text()+name;
                                type_attribute_variable.insert(insert_name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
                                type_range_variable.insert(insert_name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
                                //将变量名插入到当前模型当中
                                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                                //将变量名添加进节点中
                                program_item->appendRow(item_name);
                                //然后将变量下的对应参数插入对应的变量节点中
                                int count=0;//参入参数时候的标志
                                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                                    //获取每个参数节点的值
                                    string value=node_value->GetText();
                                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                                    //获取每个参数节点的名称
                                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                                    string name_value=value_name->Value();
                                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                                    //先插入参数节点的名称
                                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                                    //接下来将参数值的item插入到对应模型的item位置中
                                    item_name->setChild(count,1,item_value);
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ui->treeView_variable->expandToDepth(0);
}
//解析项目变量作用域xml文件，将解析的信息呈现在treeview_variable上
void Widget::set_project_range_variable_xml_update(){
    //遍历所有的项目变量xml文件
    QDir dir(root_xml_file);
    dir.setFilter(QDir::Dirs);//过滤配置，接受文件
    QFileInfoList list=dir.entryInfoList();
    QStringList filelist;
    for(int i=0;i<list.count();i++){
        QString filename=list.at(i).fileName();
        if(filename=="."||filename==".."){

        }
        else{
            QString name=filename;//获取非"."和".."的文件
            //判断当前treemodel下是否有和当前加入的项目名称同名的
            int judge_project=0;
            for(int j=0;j<root_variable->rowCount();j++){
                if(root_variable->child(j,0)){
                    QString judge_name=root_variable->child(j,0)->text();
                    QString temp_name="项目"+name;
                    if(judge_name==temp_name){
                        judge_project++;
                        break;
                    }
                }
            }
            //若为0，证明当前想插入的节点可以插入，没有重名的
            if(judge_project==0){
                QString project_name="项目"+name;
                QStandardItem* project_item=new QStandardItem(project_name);
                root_variable->appendRow(project_item);
                //插入项目节点之后，开始遍历项目文件下的项目变量
                QString path1=root_xml_file+"//"+name+"//"+name+".xml";
                //创建一个xml文档
                TiXmlDocument* doc=new TiXmlDocument;
                if(doc->LoadFile(path1.toStdString().c_str())){
                    qDebug()<<"load file success";
                }
                else{
                    qDebug()<<"load file failed";
                }
                //首先获取根节点，看根节点是否存在
                TiXmlElement* root_robot=doc->RootElement();
                if(root_robot==NULL){
                    qDebug()<<"no root";
                }
                else{
                    //开始解析节点下的子节点
                    for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                        TiXmlAttribute* node_name=node->FirstAttribute();//节点变量名
                        TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                        TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                        QString name=QString::fromStdString(node_name->Value());
                        QString insert_name=project_item->text()+name;
                        type_attribute_variable.insert(insert_name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
                        type_range_variable.insert(insert_name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
                        //将变量名插入到当前模型当中
                        QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                        //将变量名添加进节点中
                        project_item->appendRow(item_name);
                        //然后将变量下的对应参数插入对应的变量节点中
                        int count=0;//参入参数时候的标志
                        for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                            //获取每个参数节点的值
                            string value=node_value->GetText();
                            QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                            //获取每个参数节点的名称
                            TiXmlAttribute* value_name=node_value->FirstAttribute();
                            string name_value=value_name->Value();
                            QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                            //先插入参数节点的名称
                            item_name->appendRow(item_value_name);//插入名称到变量节点中
                            //接下来将参数值的item插入到对应模型的item位置中
                            item_name->setChild(count,1,item_value);
                            count++;
                        }
                    }
                }
            }
        }
    }
    ui->treeView_variable->expandToDepth(0);

}
//解析全局变量作用域xml文件，将解析的信息呈现在treeview_variable上
void Widget::set_all_range_variable_xml_update(){
    //每次执行该函数时，清空模型，便于刷新
    treemodel_variable->clear();
    treemodel_variable=new QStandardItemModel(0,2,ui->page_variable);
    treemodel_variable->setHeaderData(0,Qt::Horizontal,"变量");
    treemodel_variable->setHeaderData(1,Qt::Horizontal,"数值");
    root_variable=treemodel_variable->invisibleRootItem();//根节点，不可见
    ui->treeView_variable->setModel(treemodel_variable);
    ui->treeView_variable->setColumnWidth(0,300);
    ui->treeView_variable->setColumnWidth(1,200);
    //添加全局的节点
    QStandardItem* item_all=new QStandardItem("全局");
    root_variable->appendRow(item_all);

    //创建一个xml文档
    TiXmlDocument* doc=new TiXmlDocument;
    QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
    QStandardItem* all_item=root_variable->child(0,0);
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"load file success";
    }
    else{
        qDebug()<<"load file failed";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"no root";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            //需要判断当前xml节点的变量名，all_item下是否存在
            int judge=0;
            for(int i=0;i<all_item->rowCount();i++){
                QString now_all_name=all_item->child(i,0)->text();//当前每个节点的名称

                if(node_name->Value()==now_all_name){
                    judge++;//重名的话，递增
                    break;
                }
            }
            //如果judge还为0，证明想加入的节点名称和当前all_item没有重名
            if(judge==0){
                QString name=QString::fromStdString(node_name->Value());
                QString insert_name=all_item->text()+name;
                type_attribute_variable.insert(insert_name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
                type_range_variable.insert(insert_name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
                //将变量名插入到当前模型当中
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                //将变量名添加进节点中
                all_item->appendRow(item_name);
                //然后将变量下的对应参数插入对应的变量节点中
                int count=0;//参入参数时候的标志
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
    //使得节点的展开深度为1
    ui->treeView_variable->expandToDepth(0);
}

//变量管理模块：新建变量
void Widget::slt_variable_new_variable(){
    //新建变量，包含十种数据类型
    QModelIndex current_index=ui->treeView_variable->currentIndex();
    QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
    if(ui->comboBox_variable_type->currentText()=="<全部>"){
        //新建一个新建变量的对话框
        QDialog* Variable_dialog=new QDialog(ui->page_variable);
        Variable_dialog->setWindowTitle("变量新建");
        QFormLayout form(Variable_dialog);
        //变量的名称
        QString variable_name=QString("变量名:");
        QLineEdit* lineedit=new QLineEdit(Variable_dialog);
        form.addRow(variable_name,lineedit);
        //变量的类型，十种
        QString coordinate=QString("数据类型:");
        QComboBox* combox=new QComboBox(Variable_dialog);
        //为combox添加十种数据类型的数据
        combox->addItem("INTEGER");
        combox->addItem("FLOAT");
        combox->addItem("BOOL");
        combox->addItem("STRING");
        combox->addItem("DYNAMIC");
        combox->addItem("AXISPOS");
        combox->addItem("TOOL");
        combox->addItem("CARTREFSYS");
        combox->addItem("CARTPOS");
        combox->addItem("OVERLAP");
        form.addRow(coordinate,combox);

        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,Variable_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),Variable_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),Variable_dialog,SLOT(reject()));

        QString name;
        //当点击OK按钮时
        if(Variable_dialog->exec()==QDialog::Accepted){
            //读取用户当前所设置的变量名和数据类型
            QString name=lineedit->text();//变量名
            QString type=combox->currentText();//数据类型
            //插入容器的键
            QString insert_name=current_item->text()+name;
            //插入数据类型
            type_attribute_variable.insert(insert_name,type);
            //插入变量的作用域
            QString current_item_name=current_item->text();
            if(insert_name.left(2)=="全局"){
                type_range_variable.insert(insert_name,"全局");
            }
            if(insert_name.left(2)=="项目"){
                QString temp=current_item_name.right(current_item_name.length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            if(insert_name.left(2)=="程序"){
                QString temp=current_item_name.right(current_item_name.length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            //插入指令到对应节点中
            QStandardItem* item_variable=new QStandardItem(name);
            current_item->appendRow(item_variable);
            //插入对应数据类型的节点
            set_different_variable_type_parameter(item_variable,type);
            //生成全局变量的xml文件
            generate_all_range_variable_xml_file();
            //生成项目变量的xml文件
            generate_project_range_variable_xml_file();
            //生成程序变量的xml文件
            generate_program_range_variable_xml_file();
            //将新建变量的的值插入到var文件中
            generate_program_var_file();
        }
    }
    else{
        //新建特定类型的数据类型
        //新建一个新建变量的对话框
        QDialog* Variable_dialog=new QDialog(ui->page_variable);
        Variable_dialog->setWindowTitle("新建变量");
        QFormLayout form(Variable_dialog);
        //变量的名称
        QString variable_name=QString("变量名:");
        QLineEdit* lineedit=new QLineEdit(Variable_dialog);
        form.addRow(variable_name,lineedit);
        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,Variable_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),Variable_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),Variable_dialog,SLOT(reject()));

        QString name;
        //当点击OK按钮时
        if(Variable_dialog->exec()==QDialog::Accepted){
            //读取用户当前所设置的变量名和数据类型
            QString name=lineedit->text();//变量名
            //插入容器的键
            QString insert_name=current_item->text()+name;
            //插入数据类型
            type_attribute_variable.insert(insert_name,ui->comboBox_variable_type->currentText());
            //插入变量的作用域
            QString current_item_name=current_item->text();
            if(insert_name.left(2)=="全局"){
                type_range_variable.insert(insert_name,"全局");
            }
            if(insert_name.left(2)=="项目"){
                QString temp=current_item_name.right(current_item_name.length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            if(insert_name.left(2)=="程序"){
                QString temp=current_item_name.right(current_item_name.length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            //插入指令到对应节点中
            QStandardItem* item_variable=new QStandardItem(name);
            current_item->appendRow(item_variable);
            //插入对应数据类型的节点
            set_different_variable_type_parameter(item_variable,ui->comboBox_variable_type->currentText());
            //将该节点添加到对应的xml文件中
            set_new_variable_to_different_range_xml_file(item_variable,name);
            //将新建的变量，将指令插入到不同的var文件中
            set_new_variable_to_different_range_var_file(item_variable);
        }
    }

}
//将新建的变量名存放到相应的xml文件中
void Widget::set_new_variable_to_different_range_xml_file(QStandardItem *item, QString name){
    //首先判断作用域的类型
    QStandardItem* item_parent=item->parent();
    if(item_parent->text().left(2)=="全局"){
        QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                TiXmlElement* node=new TiXmlElement("变量节点");//设置变量名
                //获取键
                QString insert_key=item_parent->text()+name;
                node->SetAttribute("Name",name.toStdString().c_str());
                node->SetAttribute("Type",type_attribute_variable[insert_key].toStdString().c_str());
                node->SetAttribute("Range",type_range_variable[insert_key].toStdString().c_str());
                root_robot->LinkEndChild(node);
                for(int i=0;i<item->rowCount();i++){
                    if(item->child(i,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item->child(i,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                }
            }
            doc.SaveFile();
        }
    }
    if(item_parent->text().left(2)=="项目"){
        //获取项目路径的路径
        QString project_name=item_parent->text().right(item_parent->text().length()-2);
        QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                TiXmlElement* node=new TiXmlElement("变量节点");//设置变量名
                //获取键
                QString insert_key=item_parent->text()+name;
                node->SetAttribute("Name",name.toStdString().c_str());
                node->SetAttribute("Type",type_attribute_variable[insert_key].toStdString().c_str());
                node->SetAttribute("Range",type_range_variable[insert_key].toStdString().c_str());
                root_robot->LinkEndChild(node);
                for(int i=0;i<item->rowCount();i++){
                    if(item->child(i,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item->child(i,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                }
            }
            doc.SaveFile();
        }
    }
    if(item_parent->text().left(2)=="程序"){
        //获取程序路径
        //获取项目路径的路径
        QString temp=item_parent->text().right(item_parent->text().length()-2);
        QString p2=temp.left(temp.indexOf("[",1));
        QString temp1=temp.right(temp.length()-p2.length());
        QString temp2=temp1.left(temp1.length()-1);
        QString p1=temp2.right(temp2.length()-1);
        QString path=root_xml_file+"//"+p1+"//"+p2+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                TiXmlElement* node=new TiXmlElement("变量节点");//设置变量名
                //获取键
                QString insert_key=item_parent->text()+name;
                node->SetAttribute("Name",name.toStdString().c_str());
                node->SetAttribute("Type",type_attribute_variable[insert_key].toStdString().c_str());
                node->SetAttribute("Range",type_range_variable[insert_key].toStdString().c_str());
                root_robot->LinkEndChild(node);
                for(int i=0;i<item->rowCount();i++){
                    if(item->child(i,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item->child(i,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                }
            }
            doc.SaveFile();
        }
    }
}
//将新建的变量存放到相应的var文件中
void Widget::set_new_variable_to_different_range_var_file(QStandardItem *item){
    QStandardItem* item_parent=item->parent();
    QString parent_name=item_parent->text().left(2);
    //获取变量的数据类型
    QString judge_name=item_parent->text()+item->text();
    QString type=type_attribute_variable[judge_name];
    //获取对应变量的数据类型
    QString insert_format=get_different_format_var_string(item,type);
    if(parent_name=="全局"){
        //将该变量写到all的var文件中
        //获取各个项目的路径，插入指令
        QDir dir1(root_var_file);
        dir1.setFilter(QDir::Dirs);//顾虑配置，接受文件
        QFileInfoList list1=dir1.entryInfoList();
        for(int k=0;k<list1.count();k++){
            QString filename=list1.at(k).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString program_path=root_var_file+"//"+name;
                QDir dir2(program_path);
                dir2.setFilter(QDir::Files);
                QFileInfoList list2=dir2.entryInfoList();
                for(int l=0;l<list2.count();l++){
                    QString file_name=list2.at(l).fileName();
                    //获取文件的写入路径
                    QString path=root_var_file+"//"+name+"//"+file_name;
                    //将指令插入当前的var项目中
                    insert_format_value_to_var_file(path,insert_format);
                }
            }
        }
    }
    if(parent_name=="项目"){
        //获取路径
        QString project_path=item_parent->text().right(item_parent->text().length()-2);
        QString path1=root_var_file+"//"+project_path;
        //获取各项目程序的路径
        QDir dir1(path1);
        dir1.setFilter(QDir::Files);
        QFileInfoList list1=dir1.entryInfoList();
        for(int k=0;k<list1.count();k++){
            QString filename=list1.at(k).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString path=root_var_file+"//"+project_path+"//"+name;
                //将指令插入当前的var项目中
                insert_format_value_to_var_file(path,insert_format);
            }
        }
    }
    if(parent_name=="程序"){
        //获取程序路径
        QString temp=item_parent->text().right(item_parent->text().length()-2);
        QString p2=temp.left(temp.indexOf("[",1));
        QString temp1=temp.right(temp.length()-p2.length());
        QString temp2=temp1.left(temp1.length()-1);
        QString p1=temp2.right(temp2.length()-1);
        QString path=root_var_file+"//"+p1+"//"+p2+".var";
        //将指令插入当前的var项目中
        insert_format_value_to_var_file(path,insert_format);
    }
}
//变量管理模块下设置按钮的槽函数
void Widget::on_btn_variable_set_clicked()
{
    QModelIndex index=ui->treeView_variable->currentIndex();
    QStandardItem* current_item=treemodel_variable->itemFromIndex(index);
    QStandardItem* parent_item=current_item->parent();
    bool isOK;
    //创建一个临时对话框，输入程序名称并且获取用户输入的程序名称
    QString program_text=QInputDialog::getText(NULL,"Input Dialog","please input Variable_Value",QLineEdit::Normal,"Variable value",&isOK);
    //需要设计算法，求出当前索引的参数的行数
    int count_parameter;
    int count_variable;
    //计算当前程序的行数
    for(int i=0;i<parent_item->rowCount();i++){
        //若当前选中的参量==父亲item第i个参量
        if(index.data().toString()==parent_item->child(i,0)->text()){
            QStandardItem* item_value=new QStandardItem(program_text);
            parent_item->setChild(i,1,item_value);
        }
    }
    if(ui->comboBox_variable_type->currentText()=="<全部>"){
        //生成全局变量的xml文件
        generate_all_range_variable_xml_file();
        //生成项目变量的xml文件
        generate_project_range_variable_xml_file();
        //生成程序变量的xml文件
        generate_program_range_variable_xml_file();
        //将新建变量的的值插入到var文件中
        generate_program_var_file();
    }
    else{
        //将当前的设置的变量在对应的xml文件中刷新
        update_set_value_to_xml_file(current_item);
        //将新建变量的的值插入到var文件中
        generate_program_var_file();
    }

}
//将设置的值在对应的xml文件中进行刷新
void Widget::update_set_value_to_xml_file(QStandardItem *item){
    //首先判断作用域的类型
    QStandardItem* item_parent=item->parent()->parent();
    QStandardItem* variable_item=item->parent();
    QString name=variable_item->text();
    if(item_parent->text().left(2)=="全局"){
        QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                //获取各参数的值
                vector<QString> str(variable_item->rowCount());
                for(int i=0;i<variable_item->rowCount();i++){
                    if(variable_item->child(i,1)!=0){
                        str[i]=variable_item->child(i,1)->text();
                    }
                    else{
                        str[i]="0";
                    }
                }
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    int count=0;
                    if(node_name->Value()==name){
                        for(TiXmlElement* node_parameter=node->FirstChildElement();node_parameter;node_parameter=node_parameter->NextSiblingElement()){
                            TiXmlText* value=new TiXmlText(str[count].toStdString().c_str());
                            node_parameter->Clear();
                            node_parameter->LinkEndChild(value);
                            count++;
                        }
                    }
                }
            }
        }
        doc.SaveFile();
    }
    if(item_parent->text().left(2)=="项目"){
        //遍历所有项目变量的xml文件
        QString project_name=item_parent->text().right(item_parent->text().length()-2);
        QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                //获取各参数的值
                vector<QString> str(variable_item->rowCount());
                for(int i=0;i<variable_item->rowCount();i++){
                    if(variable_item->child(i,1)!=0){
                        str[i]=variable_item->child(i,1)->text();
                    }
                    else{
                        str[i]="0";
                    }
                }
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    int count=0;
                    if(node_name->Value()==name){
                        for(TiXmlElement* node_parameter=node->FirstChildElement();node_parameter;node_parameter=node_parameter->NextSiblingElement()){
                            TiXmlText* value=new TiXmlText(str[count].toStdString().c_str());
                            node_parameter->Clear();
                            node_parameter->LinkEndChild(value);
                            count++;
                        }
                    }
                }
            }
        }
        doc.SaveFile();
    }
    if(item_parent->text().left(2)=="程序"){
        //遍历所有的项目变量xml文件
        //获取项目路径的路径
        QString temp=item_parent->text().right(item_parent->text().length()-2);
        QString p2=temp.left(temp.indexOf("[",1));
        QString temp1=temp.right(temp.length()-p2.length());
        QString temp2=temp1.left(temp1.length()-1);
        QString p1=temp2.right(temp2.length()-1);
        QString path=root_xml_file+"//"+p1+"//"+p2+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                //获取各参数的值
                vector<QString> str(variable_item->rowCount());
                for(int i=0;i<variable_item->rowCount();i++){
                    if(variable_item->child(i,1)!=0){
                        str[i]=variable_item->child(i,1)->text();
                    }
                    else{
                        str[i]="0";
                    }
                }
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    int count=0;
                    if(node_name->Value()==name){
                        for(TiXmlElement* node_parameter=node->FirstChildElement();node_parameter;node_parameter=node_parameter->NextSiblingElement()){
                            TiXmlText* value=new TiXmlText(str[count].toStdString().c_str());
                            node_parameter->Clear();
                            node_parameter->LinkEndChild(value);
                            count++;
                        }
                    }
                }
            }
        }
        doc.SaveFile();
    }
}
//将新建变量的值插入到var文件中
void Widget::generate_program_var_file(){
//1
//根据变量的作用域进行插入，如果是全局的，所有程序都插入该变量
//如果是项目作用域的，该项目下的所有程序均插入该变量
//如果是程序的，只在当前程序中插入该变量
//2
//判断变量的数据类型，不同的数据类型会有不同的数据格式
    for(int i=0;i<root_variable->rowCount();i++){
        QStandardItem* range_item=root_variable->child(i,0);
        QString range_name=range_item->text().left(2);
        if(range_name=="全局"){
            //获取全局变量下的变量名
            if(range_item->rowCount()!=0){
                //证明有全局变量，获取该变量的名称
                for(int j=0;j<range_item->rowCount();j++){
                    QStandardItem* all_item=range_item->child(j,0);
                    QString all_item_name=all_item->text();
                    //获取每一个变量的数据类型，作用域不需要提取
                    QString judge_name=range_name+all_item_name;
                    QString all_item_type=type_attribute_variable[judge_name];
                    //获取不同数据类型的插入格式
                    QString insert_format=get_different_format_var_string(all_item,all_item_type);
                    //获取各个项目的路径，插入指令
                    QDir dir1(root_var_file);
                    dir1.setFilter(QDir::Dirs);//顾虑配置，接受文件
                    QFileInfoList list1=dir1.entryInfoList();
                    for(int k=0;k<list1.count();k++){
                        QString filename=list1.at(k).fileName();
                        if(filename=="."||filename==".."){

                        }
                        else{
                            QString name=filename;//获取非"."文件
                            QString program_path=root_var_file+"//"+name;
                            QDir dir2(program_path);
                            dir2.setFilter(QDir::Files);
                            QFileInfoList list2=dir2.entryInfoList();
                            for(int l=0;l<list2.count();l++){
                                QString file_name=list2.at(l).fileName();
                                //获取文件的写入路径
                                QString path=root_var_file+"//"+name+"//"+file_name;
                                //将指令插入当前的var项目中
                                insert_format_value_to_var_file(path,insert_format);
                            }
                        }
                    }
                }
            }
        }
        if(range_name=="项目"){
            //获取项目变量下的变量名
            if(range_item->rowCount()!=0){
                //证明有项目变量，获取该变量的名称
                for(int j=0;j<range_item->rowCount();j++){
                    QStandardItem* project_item=range_item->child(j,0);
                    QString project_name=project_item->text();
                    //获取每一个变量的数据类型，作用域不需要提取
                    QString judge_name=range_item->text()+project_name;
                    QString project_item_type=type_attribute_variable[judge_name];
                    //获取不同数据类型的插入格式
                    QString insert_format=get_different_format_var_string(project_item,project_item_type);
                    //获取各个项目的路径，插入指令
                    QString project_path=range_item->text().right(range_item->text().length()-2);
                    QString path1=root_var_file+"//"+project_path;
                    //获取各项目程序的路径
                    QDir dir1(path1);
                    dir1.setFilter(QDir::Files);
                    QFileInfoList list1=dir1.entryInfoList();
                    for(int k=0;k<list1.count();k++){
                        QString filename=list1.at(k).fileName();
                        if(filename=="."||filename==".."){

                        }
                        else{
                            QString name=filename;//获取非"."文件
                            QString path=root_var_file+"//"+project_path+"//"+name;
                            //将指令插入当前的var项目中
                            insert_format_value_to_var_file(path,insert_format);
                        }
                    }



                }
            }

        }
        if(range_name=="程序"){
            //获取程序变量下的变量名
            if(range_item->rowCount()!=0){
                //证明有程序变量，获取该变量的名字
                for(int j=0;j<range_item->rowCount();j++){
                    QStandardItem* program_item=range_item->child(j,0);
                    QString program_name=program_item->text();
                    //获取每一个变量类型的数据类型，作用域不提取
                    QString judge_name=range_item->text()+program_name;
                    QString program_item_type=type_attribute_variable[judge_name];
                    //获取不同数据类型的格式
                    QString insert_format=get_different_format_var_string(program_item,program_item_type);
                    //获取各程序路径，插入指令
                    QString temp=range_item->text().right(range_item->text().length()-2);
                    QString p2=temp.left(temp.indexOf("[",1));
                    QString temp1=temp.right(temp.length()-p2.length());
                    QString temp2=temp1.left(temp1.length()-1);
                    QString p1=temp2.right(temp2.length()-1);
                    QString path=root_var_file+"//"+p1+"//"+p2+".var";
                    //将指令插入当前的var项目中
                    insert_format_value_to_var_file(path,insert_format);
                }
            }

        }

    }

}
//将当前要插入的指令插入var文件中
void Widget::insert_format_value_to_var_file(QString path, QString value){

    QFile file(path);//获取当前程序的路径

    if(file.size()==0){
        //如果var文件为空，就是第一条指令插入的时候执行的条件
        file.open(QIODevice::WriteOnly|QIODevice::Text);
        file.write("{");
        file.write("\n");
        file.write(value.toStdString().c_str());
        file.write("\n");
        file.write("}");
        file.close();
    }
    else{
        //获取想插入指令的变量名
        QString judge_name=value.left(value.indexOf(":",1));
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        //首先读取当前文件的内容
        QTextStream in(&file);
        in.setCodec(QTextCodec::codecForName("UTF-8"));
        QStringList list;
        while(!in.atEnd()){
            QString line=in.readLine();
            list.append(line);
        }
        file.close();

        int count=0;//索引
        for(int i=1;i<list.size()-1;i++){
            QString temp=list[i].left(list[i].indexOf(":",1));

            if(temp==judge_name){
                //存在重名，新插入的值刷新旧的值
                int a=list.indexOf(list[i],1);
                list.replace(a,value);
                count++;
            }
        }
        if(count==0){
            //证明没有重名的，插入新的指令
            list.insert(1,value);
        }

        //将stringlist中的值写入文本中
        QFile file2(path);
        file2.open(QFile::WriteOnly|QFile::Truncate);
        QTextStream out(&file2);
        out.setCodec(QTextCodec::codecForName("UTF-8"));
        for(int i=0;i<list.size();i++){
            out<<list[i];
            out<<"\n";
        }

    }

}
//返回不同数据类型插入var文件的变量格式
QString Widget::get_different_format_var_string(QStandardItem* root,QString type){
    //根据不同的数据类型，返回不同的var变量格式
    vector<QString> str(root->rowCount());
    //获取当前item的各个参数的数值
    for(int i=0;i<root->rowCount();i++){
        if(root->child(i,1)!=0){
            str[i]=root->child(i,1)->text();
        }
        else{
            if(type=="CARTREFSYS"){
                str[0]="worldRefSys";
            }
            str[i]="0";
        }
    }

    if(type=="INTEGER"){
        QString value=root->text()+":"+type+"="+str[0];
        return value;
    }
    if(type=="FLOAT"){
        QString value=root->text()+":"+type+"="+str[0];
        return  value;
    }
    if(type=="BOOL"){
        QString value=root->text()+":"+type+"="+str[0];
        return value;
    }
    if(type=="STRING"){
        QString value=root->text()+":"+type+"="+str[0];
        return value;
    }
    if(type=="DYNAMIC"){
        QString value=root->text()+":"+type+"=("+"velAxis="+str[0]+",accAxis="+str[1]+",decAxis="+str[2]+",jerkAxis="+str[3]+",velPath="+str[4]+",accPath="+str[5]+",decPath="+str[6]+",jerkPath="+str[7]+",velOri="+str[8]+",accOri="+str[9]+",decOri="+str[10]+",jerkOri="+str[11]+")";
        return value;
    }
    if(type=="AXISPOS"){
        QString value=root->text()+":"+type+"=(a1="+str[0]+",a2="+str[1]+",a3="+str[2]+",a4="+str[3]+",a5="+str[4]+",a6="+str[5]+")";
        return value;
    }
    if(type=="TOOL"){
        QString value=root->text()+":"+type+"=(x="+str[0]+",y="+str[1]+",z="+str[2]+",a="+str[3]+",b="+str[4]+",c="+str[5]+")";
        return  value;
    }
    if(type=="CARTREFSYS"){
        QString value=root->text()+":"+type+"=(baseRefSys="+str[0]+",x="+str[1]+",y="+str[2]+",z="+str[3]+",a="+str[4]+",b="+str[5]+",c="+str[6]+")";
        return value;
    }
    if(type=="CARTPOS"){
        QString value=root->text()+":"+type+"=(x="+str[0]+",y="+str[1]+",z="+str[2]+",a="+str[3]+",b="+str[4]+",c="+str[5]+",mode="+str[6]+")";
        return value;
    }
    if(type=="OVERLAP"){
        QString value=root->text()+":"+type+"=(posDist="+str[0]+",oriDist="+str[1]+",linAxDist="+str[2]+",rotAxDist="+str[3]+")";
        return value;
    }

}
//生成全局变量的xml文件
void Widget::generate_all_range_variable_xml_file(){
    //创建一个xml文档
    TiXmlDocument doc;
    //添加文档头部的基本声明
    TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","UTF-8","yes");
    doc.LinkEndChild(dec);
    //为文档添加基本的注释
    TiXmlComment* comment=new TiXmlComment("This is Robot_Variable Information");
    doc.LinkEndChild(comment);
    //为xml文档添加根元素
    TiXmlElement* root_robot=new TiXmlElement("config_information");

    //开始写数据进xml文件中
    QStandardItem* all_item=root_variable->child(0);//首先获取全局item
    if(all_item->rowCount()){
        //不为零，证明有全局变量
        vector<TiXmlElement*> node_all_range(all_item->rowCount());
        for(int i=0;i<all_item->rowCount();i++){
            //获取每个变量名的索引
            QString variable_name=all_item->child(i,0)->text();
            QString name=all_item->text()+variable_name;//键
            //插入新建的变量节点
            node_all_range[i]=new TiXmlElement("变量节点");
            //设置变量节点的数据类型和作用域
            node_all_range[i]->SetAttribute("Name",variable_name.toStdString().c_str());
            node_all_range[i]->SetAttribute("Type",type_attribute_variable[name].toStdString().c_str());
            node_all_range[i]->SetAttribute("Range",type_range_variable[name].toStdString().c_str());
            root_robot->LinkEndChild(node_all_range[i]);
            for(int j=0;j<all_item->child(i,0)->rowCount();j++){
                if(all_item->child(i,0)->child(j,1)!=0){
                    //如果变量名下的参数值不为0，将当前值写入节点中
                    TiXmlElement* value=new TiXmlElement("value");
                    value->SetAttribute("Parameter",all_item->child(i,0)->child(j,0)->text().toStdString().c_str());
                    TiXmlText* value_text=new TiXmlText(all_item->child(i,0)->child(j,1)->text().toStdString().c_str());
                    value->LinkEndChild(value_text);
                    node_all_range[i]->LinkEndChild(value);
                }
                else{

                    //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                    TiXmlElement* value=new TiXmlElement("value");
                    value->SetAttribute("Parameter",all_item->child(i,0)->child(j,0)->text().toStdString().c_str());
                    TiXmlText* value_text=new TiXmlText("0");
                    value->LinkEndChild(value_text);
                    node_all_range[i]->LinkEndChild(value);
                }
            }
        }
    }
    doc.LinkEndChild(root_robot);//插入根节点
    //获取程序保存路径
    QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
    doc.SaveFile(path.toStdString().c_str());
}
//生成项目变量的xml文件
void Widget::generate_project_range_variable_xml_file(){
    //首先遍历treeview_variable上的项目节点，筛选出来
    for(int i=0;i<root_variable->rowCount();i++){
        QString range_name=root_variable->child(i,0)->text();
        if(range_name!="全局"){
            QString judge=range_name.left(2);
            if(judge=="项目"){
                //获取各个项目xml文件的路径
                QString project_name=range_name.right(range_name.length()-2);
                QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";
                //创建一个xml文档
                TiXmlDocument doc;
                //添加文档头部声明
                TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","UTF-8","yes");
                doc.LinkEndChild(dec);
                //为文档添加基本的注释
                TiXmlComment* comment=new TiXmlComment("This is Robot_Variable Information");
                doc.LinkEndChild(comment);
                //为xml文档添加根元素
                TiXmlElement* root_robot=new TiXmlElement("config_information");

                //开始写数据进xml文件中
                QStandardItem* project_item=root_variable->child(i);//获取项目item
                if(project_item->rowCount()){
                    //不为零，证明有项目变量
                    vector<TiXmlElement*> node_project_range(project_item->rowCount());
                    for(int j=0;j<project_item->rowCount();j++){
                        //获取每个变量名的索引
                        QString variable_name=project_item->child(j,0)->text();
                        QString name=project_item->text()+variable_name;//键
                        //插入新建的变量节点
                        node_project_range[j]=new TiXmlElement("变量节点");
                        //设置变量节点的数据类型和作用域
                        node_project_range[j]->SetAttribute("Name",variable_name.toStdString().c_str());
                        node_project_range[j]->SetAttribute("Type",type_attribute_variable[name].toStdString().c_str());
                        node_project_range[j]->SetAttribute("Range",type_range_variable[name].toStdString().c_str());
                        root_robot->LinkEndChild(node_project_range[j]);
                        for(int k=0;k<project_item->child(j,0)->rowCount();k++){
                            if(project_item->child(j,0)->child(k,1)!=0){
                                //如果变量名下的参数不为0，将当前值写入节点中
                                TiXmlElement* value=new TiXmlElement("value");
                                value->SetAttribute("Parameter",project_item->child(j,0)->child(k,0)->text().toStdString().c_str());
                                TiXmlText* value_text=new TiXmlText(project_item->child(j,0)->child(k,1)->text().toStdString().c_str());
                                value->LinkEndChild(value_text);
                                node_project_range[j]->LinkEndChild(value);
                            }
                            else{
                                //若当前变量名下的参数值为0，将自动赋值为0，写入节点中
                                TiXmlElement* value=new TiXmlElement("value");
                                value->SetAttribute("Parameter",project_item->child(j,0)->child(k,0)->text().toStdString().c_str());
                                TiXmlText* value_text=new TiXmlText("0");
                                value->LinkEndChild(value_text);
                                node_project_range[j]->LinkEndChild(value);
                            }
                        }
                    }
                }
                doc.LinkEndChild(root_robot);//插入根节点
                doc.SaveFile(path.toStdString().c_str());
            }
        }
    }
}
//生成程序变量的xml文件
void Widget::generate_program_range_variable_xml_file(){
    //筛选程序变量Item
    for(int i=0;i<root_variable->rowCount();i++){
        QString range_name=root_variable->child(i,0)->text();
        if(range_name!="全局"){
            QString judge=range_name.left(2);
            if(judge=="程序"){
                //获取各个程序的xml文件路径
                QString temp=range_name.right(range_name.length()-2);
                QString program_name=temp.left(temp.indexOf("[",1));
                QString temp2=temp.right(temp.length()-temp.indexOf("[",1));
                QString temp3=temp2.left(temp2.length()-1);
                QString project_name=temp3.right(temp3.length()-1);
                QString path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
                //创建一个文档
                TiXmlDocument doc;
                //添加文档头部声明
                TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","UTF-8","yes");
                doc.LinkEndChild(dec);
                //为文档添加基本的注释
                TiXmlComment* comment=new TiXmlComment("This is Robot_Variable Information");
                doc.LinkEndChild(comment);
                //为xml文档添加根元素
                TiXmlElement* root_robot=new TiXmlElement("config_information");

                //开始写数据进xml文件中
                QStandardItem* program_item=root_variable->child(i);//获取程序item
                if(program_item->rowCount()){
                    //不为零，证明有程序变量
                    vector<TiXmlElement*> node_program_range(program_item->rowCount());
                    for(int j=0;j<program_item->rowCount();j++){
                        //获取每个变量名的索引
                        QString variable_name=program_item->child(j,0)->text();
                        QString name=program_item->text()+variable_name;//键
                        //插入新建的变量节点
                        node_program_range[j]=new TiXmlElement("变量节点");
                        //设置变量节点的数据类型和作用域
                        node_program_range[j]->SetAttribute("Name",variable_name.toStdString().c_str());
                        node_program_range[j]->SetAttribute("Type",type_attribute_variable[name].toStdString().c_str());
                        node_program_range[j]->SetAttribute("Range",type_range_variable[name].toStdString().c_str());
                        root_robot->LinkEndChild(node_program_range[j]);
                        for(int k=0;k<program_item->child(j,0)->rowCount();k++){
                            if(program_item->child(j,0)->child(k,1)!=0){
                                //如果变量名下的参数不为0，将当前值写入节点中
                                TiXmlElement* value=new TiXmlElement("value");
                                value->SetAttribute("Parameter",program_item->child(j,0)->child(k,0)->text().toStdString().c_str());
                                TiXmlText* value_text=new TiXmlText(program_item->child(j,0)->child(k,1)->text().toStdString().c_str());
                                value->LinkEndChild(value_text);
                                node_program_range[j]->LinkEndChild(value);
                            }
                            else{
                                //若当前变量名下的参数值为0，将自动赋值为0，写入节点中
                                TiXmlElement* value=new TiXmlElement("value");
                                value->SetAttribute("Parameter",program_item->child(j,0)->child(k,0)->text().toStdString().c_str());
                                TiXmlText* value_text=new TiXmlText("0");
                                value->LinkEndChild(value_text);
                                node_program_range[j]->LinkEndChild(value);
                            }
                        }
                    }
                }
                doc.LinkEndChild(root_robot);//插入根节点
                doc.SaveFile(path.toStdString().c_str());
            }
        }
    }
}
//根据不同的数据类型，插入不同的参数到变量节点中
void Widget::set_different_variable_type_parameter(QStandardItem* item_variable,QString type_name){
    if(type_name=="INTEGER"){
        set_only_one_parameter(item_variable,"整数数值");
    }
    if(type_name=="FLOAT"){
        set_only_one_parameter(item_variable,"浮点数数值");
    }
    if(type_name=="BOOL"){
        set_only_one_parameter(item_variable,"布尔值");
    }
    if(type_name=="STRING"){
        set_only_one_parameter(item_variable,"字符串");
    }
    if(type_name=="DYNAMIC"){
        set_dyn_parameter(item_variable);
        //展开该新建的节点
        QModelIndex new_index=item_variable->index();
        ui->treeView_variable->expand(new_index);
    }
    if(type_name=="AXISPOS"){
        set_pos_AXIPOS_parameter(item_variable);
        //展开该新建的节点
        QModelIndex new_index=item_variable->index();
        ui->treeView_variable->expand(new_index);
    }
    if(type_name=="TOOL"){
        set_TOOL_parameter(item_variable);
    }
    if(type_name=="CARTREFSYS"){
        set_CARTREFSYS_parameter(item_variable);
    }
    if(type_name=="CARTPOS"){
        set_pos_CARTPOS_parameter(item_variable);
        //展开该新建的节点
        QModelIndex new_index=item_variable->index();
        ui->treeView_variable->expand(new_index);
    }
    if(type_name=="OVERLAP"){
        set_OVERLAP_parameter(item_variable);
    }
}
//插入单一参数的变量
void Widget::set_only_one_parameter(QStandardItem *item_variable, QString name){
    QStandardItem* item1=new QStandardItem(name);
    item_variable->appendRow(item1);
    //展开新建的节点
    QModelIndex new_index=item_variable->index();
    ui->treeView_variable->expand(new_index);
}
//插入TOOL参数的变量
void Widget::set_TOOL_parameter(QStandardItem *item_variable){
    QStandardItem* item_x=new QStandardItem("x");
    QStandardItem* item_y=new QStandardItem("y");
    QStandardItem* item_z=new QStandardItem("z");
    QStandardItem* item_a=new QStandardItem("a");
    QStandardItem* item_b=new QStandardItem("b");
    QStandardItem* item_c=new QStandardItem("c");
    item_variable->appendRow(item_x);
    item_variable->appendRow(item_y);
    item_variable->appendRow(item_z);
    item_variable->appendRow(item_a);
    item_variable->appendRow(item_b);
    item_variable->appendRow(item_c);
    //展开该新建的节点
    QModelIndex new_index=item_variable->index();
    ui->treeView_variable->expand(new_index);
}
//插入CARTREFSYS参数的变量
void Widget::set_CARTREFSYS_parameter(QStandardItem *item_variable){
    QStandardItem* item_1=new QStandardItem("baseRefSys");
    QStandardItem* item_3=new QStandardItem("x");
    QStandardItem* item_4=new QStandardItem("y");
    QStandardItem* item_5=new QStandardItem("z");
    QStandardItem* item_6=new QStandardItem("a");
    QStandardItem* item_7=new QStandardItem("b");
    QStandardItem* item_8=new QStandardItem("c");
    item_variable->appendRow(item_1);
    item_variable->appendRow(item_3);
    item_variable->appendRow(item_4);
    item_variable->appendRow(item_5);
    item_variable->appendRow(item_6);
    item_variable->appendRow(item_7);
    item_variable->appendRow(item_8);
    QStandardItem* item_refsys=new QStandardItem("worldRefSys");
    item_variable->setChild(0,1,item_refsys);
    //展开该新建的节点
    QModelIndex new_index=item_variable->index();
    ui->treeView_variable->expand(new_index);
}
//插入OVERLAP参数的变量
void Widget::set_OVERLAP_parameter(QStandardItem *item_variable){
    QStandardItem* item_1=new QStandardItem("posDist");
    QStandardItem* item_2=new QStandardItem("oriDist");
    QStandardItem* item_3=new QStandardItem("linAxDist");
    QStandardItem* item_4=new QStandardItem("rotAxDist");
    item_variable->appendRow(item_1);
    item_variable->appendRow(item_2);
    item_variable->appendRow(item_3);
    item_variable->appendRow(item_4);
    //展开该新建的节点
    QModelIndex new_index=item_variable->index();
    ui->treeView_variable->expand(new_index);
}
//变量管理模块：删除变量
void Widget::slt_variable_delete_variable(){

    //选择某个变量，删除该节点变量
    QModelIndex current_index=ui->treeView_variable->currentIndex();
    QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
    QStandardItem* item_parent=current_item->parent();
    //根据程序路径，删除对应的var文件中对应变量的信息
    delete_different_range_variable_var(item_parent);
    if(ui->comboBox_variable_type->currentText()=="<全部>"){
        //删除当前视图上对应的节点
        for(int i=0;i<item_parent->rowCount();i++){
            if(item_parent->child(i,0)->index().data().toString()==current_index.data().toString()){
                QString delete_name=item_parent->text()+current_item->text();
                QMap<QString,QString>::Iterator it;
                QMap<QString,QString>::Iterator it2;
                it=type_range_variable.find(delete_name);
                it2=type_attribute_variable.find(delete_name);
                type_range_variable.erase(it);
                type_attribute_variable.erase(it2);
                item_parent->removeRow(i);
            }
        }
        //删除完变量之后，刷新数据到文件中
        generate_all_range_variable_xml_file();//全局
        generate_project_range_variable_xml_file();//项目
        generate_program_range_variable_xml_file();//程序
    }
    else{
        delete_different_range_variable_xml(current_item);//删除选中变量在对应xml文件的信息
        //删除当前视图上对应的节点
        for(int i=0;i<item_parent->rowCount();i++){
            if(item_parent->child(i,0)->index().data().toString()==current_index.data().toString()){
                QString delete_name=item_parent->text()+current_item->text();
                QMap<QString,QString>::Iterator it;
                QMap<QString,QString>::Iterator it2;
                it=type_range_variable.find(delete_name);
                it2=type_attribute_variable.find(delete_name);
                type_range_variable.erase(it);
                type_attribute_variable.erase(it2);
                item_parent->removeRow(i);
            }
        }
    }
}
//根据程序路径，删除对应的var文件中对应变量的信息
void Widget::delete_different_range_variable_var(QStandardItem *item_parent){
    //选择某个变量，删除该节点变量
    QModelIndex current_index=ui->treeView_variable->currentIndex();
    QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
    QString delete_name=current_item->text();
    //删除变量，需要对var文件进行处理
    //根据不同的
    if(item_parent->text().left(2)=="全局"){
        //获取各个项目的路径
        QDir dir1(root_var_file);
        dir1.setFilter(QDir::Dirs);//顾虑配置，接受文件
        QFileInfoList list1=dir1.entryInfoList();
        for(int i=0;i<list1.count();i++){
            QString filename=list1.at(i).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString program_path=root_var_file+"//"+name;
                QDir dir2(program_path);
                dir2.setFilter(QDir::Files);
                QFileInfoList list2=dir2.entryInfoList();
                for(int l=0;l<list2.count();l++){
                    QString file_name=list2.at(l).fileName();
                    //获取文件的写入路径
                    QString path=root_var_file+"//"+name+"//"+file_name;
                    //从当前程序中删除选中的指令
                    delete_current_choose_variable_var(path,delete_name);
                }

            }
        }

    }
    if(item_parent->text().left(2)=="项目"){
        //获取项目下各个程序的路径
        QString project_path=item_parent->text().right(item_parent->text().length()-2);
        QString path1=root_var_file+"//"+project_path;
        QDir dir1(path1);
        dir1.setFilter(QDir::Files);
        QFileInfoList list1=dir1.entryInfoList();
        for(int k=0;k<list1.count();k++){
            QString filename=list1.at(k).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString path=root_var_file+"//"+project_path+"//"+name;
                //从当前程序中删除选中的指令
                delete_current_choose_variable_var(path,delete_name);
            }
        }
    }
    if(item_parent->text().left(2)=="程序"){
        //获取程序路径
        QString temp=item_parent->text().right(item_parent->text().length()-2);
        QString p2=temp.left(temp.indexOf("[",1));
        QString temp1=temp.right(temp.length()-p2.length());
        QString temp2=temp1.left(temp1.length()-1);
        QString p1=temp2.right(temp2.length()-1);
        QString path=root_var_file+"//"+p1+"//"+p2+".var";
        delete_current_choose_variable_var(path,delete_name);
    }

}
//根据程序路径，删除对应的xml文件中对应变量的信息
void Widget::delete_different_range_variable_xml(QStandardItem *item){
    //根据不同的路径删除变量
    QStandardItem* item_parent=item->parent();
    QString name=item->text();
    if(item_parent->text().left(2)=="全局"){
        QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    if(node_name->Value()==name){
                        root_robot->RemoveChild(node);
                    }
                }
            }
        }
        doc.SaveFile();
    }
    if(item_parent->text().left(2)=="项目"){
        //遍历所有项目变量的xml文件
        QString project_name=item_parent->text().right(item_parent->text().length()-2);
        QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    if(node_name->Value()==name){
                        root_robot->RemoveChild(node);
                    }
                }
            }
        }
        doc.SaveFile();
    }
    if(item_parent->text().left(2)=="程序"){
        //遍历所有的项目变量xml文件
        //获取项目路径的路径
        QString temp=item_parent->text().right(item_parent->text().length()-2);
        QString p2=temp.left(temp.indexOf("[",1));
        QString temp1=temp.right(temp.length()-p2.length());
        QString temp2=temp1.left(temp1.length()-1);
        QString p1=temp2.right(temp2.length()-1);
        QString path=root_xml_file+"//"+p1+"//"+p2+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    if(node_name->Value()==name){
                        root_robot->RemoveChild(node);
                    }
                }
            }
        }
        doc.SaveFile();
    }
}

//变量管理模块：复制变量
void Widget::slt_variable_copy_variable(){
    //获取当前的item
    QModelIndex current_index=ui->treeView_variable->currentIndex();
    QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
    QStandardItem* patent_item=current_item->parent();
    //将当前的item赋给全局变量copy_item
    copy_item=current_item;
}
//变量管理模块：粘贴变量
void Widget::slt_variable_paste_variable(){
    if(ui->comboBox_variable_type->currentText()=="<全部>"){
        if(copy_item->text().right(3)=="cut"){
            //删除cut_item的var文件信息
            QString cut_name=copy_item->text().left(copy_item->text().length()-3);
            //根据不同的变量作用域删除var文件
            if(copy_item->parent()->text().left(2)=="全局"){
                //获取各个项目的路径
                QDir dir1(root_var_file);
                dir1.setFilter(QDir::Dirs);//顾虑配置，接受文件
                QFileInfoList list1=dir1.entryInfoList();
                for(int i=0;i<list1.count();i++){
                    QString filename=list1.at(i).fileName();
                    if(filename=="."||filename==".."){

                    }
                    else{
                        QString name=filename;//获取非"."文件
                        QString program_path=root_var_file+"//"+name;
                        QDir dir2(program_path);
                        dir2.setFilter(QDir::Files);
                        QFileInfoList list2=dir2.entryInfoList();
                        for(int l=0;l<list2.count();l++){
                            QString file_name=list2.at(l).fileName();
                            //获取文件的写入路径
                            QString path=root_var_file+"//"+name+"//"+file_name;
                            //从当前程序中删除选中的指令
                            delete_current_choose_variable_var(path,cut_name);
                        }
                    }
                }
            }
            if(copy_item->parent()->text().left(2)=="项目"){
                //获取项目下各个程序的路径
                QString project_path=copy_item->parent()->text().right(copy_item->parent()->text().length()-2);
                QString path1=root_var_file+"//"+project_path;
                QDir dir1(path1);
                dir1.setFilter(QDir::Files);
                QFileInfoList list1=dir1.entryInfoList();
                for(int k=0;k<list1.count();k++){
                    QString filename=list1.at(k).fileName();
                    if(filename=="."||filename==".."){

                    }
                    else{
                        QString name=filename;//获取非"."文件
                        QString path=root_var_file+"//"+project_path+"//"+name;
                        //从当前程序中删除选中的指令
                        delete_current_choose_variable_var(path,cut_name);
                    }
                }

            }
            if(copy_item->parent()->text().left(2)=="程序"){
                //获取程序路径
                QString temp=copy_item->parent()->text().right(copy_item->parent()->text().length()-2);
                QString p2=temp.left(temp.indexOf("[",1));
                QString temp1=temp.right(temp.length()-p2.length());
                QString temp2=temp1.left(temp1.length()-1);
                QString p1=temp2.right(temp2.length()-1);
                QString path=root_var_file+"//"+p1+"//"+p2+".var";
                delete_current_choose_variable_var(path,cut_name);
            }
            //获取copy_item的数据类型
            //获取copy_item的数据类型
            QString search_name=copy_item->parent()->text()+copy_item->text().left(copy_item->text().length()-3);
            QString copy_item_type=type_attribute_variable[search_name];//数据类型

            //获取当前想插入的item
            QModelIndex current_index=ui->treeView_variable->currentIndex();
            QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
            //获取粘贴的变量名
            QString paste_variable_name=copy_item->text().left(copy_item->text().length()-3);
            QStandardItem* paste_variable_item=new QStandardItem(paste_variable_name);
            //将复制得到的item添加到现在的current_item上面
            current_item->appendRow(paste_variable_item);
            for(int i=0;i<copy_item->rowCount();i++){
                QString parameter_name=copy_item->child(i,0)->text();
                QStandardItem* name_item=new QStandardItem(parameter_name);
                if(copy_item->child(i,1)!=0){
                    QString parameter_value=copy_item->child(i,1)->text();
                    QStandardItem* value_item=new QStandardItem(parameter_value);
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
                else{
                    QStandardItem* value_item=new QStandardItem("0");
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
            }
            //插入copy_item的数据类型
            QString insert_name=current_item->text()+paste_variable_name;
            type_attribute_variable.insert(insert_name,copy_item_type);
            //插入copy_item的
            if(current_item->text().left(2)=="全局"){
                type_range_variable.insert(insert_name,"全局");
            }
            if(current_item->text().left(2)=="项目"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            if(current_item->text().left(2)=="程序"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            //删除其他item中的和copy_item重名的item
            for(int i=0;i<copy_item->parent()->rowCount();i++){
                if(copy_item->parent()->child(i,0)->index().data().toString()==copy_item->text()){
                    QString delete_name=copy_item->parent()->text()+copy_item->text().left(copy_item->text().length()-3);
                    QMap<QString,QString>::Iterator it;
                    QMap<QString,QString>::Iterator it2;
                    it=type_range_variable.find(delete_name);
                    it2=type_attribute_variable.find(delete_name);
                    type_range_variable.erase(it);
                    type_attribute_variable.erase(it2);
                    copy_item->parent()->removeRow(i);
                }
            }
        }
        else{
            //获取copy_item的数据类型
            QString search_name=copy_item->parent()->text()+copy_item->text();
            QString copy_item_type=type_attribute_variable[search_name];//数据类型

            //获取当前想插入的item
            QModelIndex current_index=ui->treeView_variable->currentIndex();
            QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
            //获取粘贴的变量名
            QString paste_variable_name=copy_item->text();
            QStandardItem* paste_variable_item=new QStandardItem(paste_variable_name);
            //将复制得到的item添加到现在的current_item上面
            current_item->appendRow(paste_variable_item);
            for(int i=0;i<copy_item->rowCount();i++){
                QString parameter_name=copy_item->child(i,0)->text();
                QStandardItem* name_item=new QStandardItem(parameter_name);
                if(copy_item->child(i,1)!=0){
                    QString parameter_value=copy_item->child(i,1)->text();
                    QStandardItem* value_item=new QStandardItem(parameter_value);
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
                else{
                    QStandardItem* value_item=new QStandardItem("0");
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
            }
            //插入copy_item的数据类型
            QString insert_name=current_item->text()+paste_variable_name;
            type_attribute_variable.insert(insert_name,copy_item_type);
            //插入copy_item的
            if(current_item->text().left(2)=="全局"){
                type_range_variable.insert(insert_name,"全局");
            }
            if(current_item->text().left(2)=="项目"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            if(current_item->text().left(2)=="程序"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
        }
        //生成对应的全局变量xml文件
        generate_all_range_variable_xml_file();
        //生成项目变量的xml文件
        generate_project_range_variable_xml_file();
        //生成程序变量的xml文件
        generate_program_range_variable_xml_file();
        //将新建变量的的值插入到var文件中
        generate_program_var_file();
    }
    else{
        if(copy_item->text().right(3)=="cut"){
            //删除cut_item的var文件信息
            QString cut_name=copy_item->text().left(copy_item->text().length()-3);
            //根据不同的变量作用域删除var文件
            if(copy_item->parent()->text().left(2)=="全局"){
                //获取各个项目的路径
                QDir dir1(root_var_file);
                dir1.setFilter(QDir::Dirs);//顾虑配置，接受文件
                QFileInfoList list1=dir1.entryInfoList();
                for(int i=0;i<list1.count();i++){
                    QString filename=list1.at(i).fileName();
                    if(filename=="."||filename==".."){

                    }
                    else{
                        QString name=filename;//获取非"."文件
                        QString program_path=root_var_file+"//"+name;
                        QDir dir2(program_path);
                        dir2.setFilter(QDir::Files);
                        QFileInfoList list2=dir2.entryInfoList();
                        for(int l=0;l<list2.count();l++){
                            QString file_name=list2.at(l).fileName();
                            //获取文件的写入路径
                            QString path=root_var_file+"//"+name+"//"+file_name;
                            //从当前程序中删除选中的指令
                            delete_current_choose_variable_var(path,cut_name);
                        }
                    }
                }
            }
            if(copy_item->parent()->text().left(2)=="项目"){
                //获取项目下各个程序的路径
                QString project_path=copy_item->parent()->text().right(copy_item->parent()->text().length()-2);
                QString path1=root_var_file+"//"+project_path;
                QDir dir1(path1);
                dir1.setFilter(QDir::Files);
                QFileInfoList list1=dir1.entryInfoList();
                for(int k=0;k<list1.count();k++){
                    QString filename=list1.at(k).fileName();
                    if(filename=="."||filename==".."){

                    }
                    else{
                        QString name=filename;//获取非"."文件
                        QString path=root_var_file+"//"+project_path+"//"+name;
                        //从当前程序中删除选中的指令
                        delete_current_choose_variable_var(path,cut_name);
                    }
                }

            }
            if(copy_item->parent()->text().left(2)=="程序"){
                //获取程序路径
                QString temp=copy_item->parent()->text().right(copy_item->parent()->text().length()-2);
                QString p2=temp.left(temp.indexOf("[",1));
                QString temp1=temp.right(temp.length()-p2.length());
                QString temp2=temp1.left(temp1.length()-1);
                QString p1=temp2.right(temp2.length()-1);
                QString path=root_var_file+"//"+p1+"//"+p2+".var";
                delete_current_choose_variable_var(path,cut_name);
            }
            //获取copy_item的数据类型
            //获取copy_item的数据类型
            QString search_name=copy_item->parent()->text()+copy_item->text().left(copy_item->text().length()-3);
            QString copy_item_type=type_attribute_variable[search_name];//数据类型

            //获取当前想插入的item
            QModelIndex current_index=ui->treeView_variable->currentIndex();
            QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
            //获取粘贴的变量名
            QString paste_variable_name=copy_item->text().left(copy_item->text().length()-3);
            QStandardItem* paste_variable_item=new QStandardItem(paste_variable_name);
            //将复制得到的item添加到现在的current_item上面
            current_item->appendRow(paste_variable_item);
            for(int i=0;i<copy_item->rowCount();i++){
                QString parameter_name=copy_item->child(i,0)->text();
                QStandardItem* name_item=new QStandardItem(parameter_name);
                if(copy_item->child(i,1)!=0){
                    QString parameter_value=copy_item->child(i,1)->text();
                    QStandardItem* value_item=new QStandardItem(parameter_value);
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
                else{
                    QStandardItem* value_item=new QStandardItem("0");
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
            }
            //插入copy_item的数据类型
            QString insert_name=current_item->text()+paste_variable_name;
            type_attribute_variable.insert(insert_name,copy_item_type);
            //插入copy_item的
            if(current_item->text().left(2)=="全局"){
                type_range_variable.insert(insert_name,"全局");
            }
            if(current_item->text().left(2)=="项目"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            if(current_item->text().left(2)=="程序"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            //将该节点添加到对应的xml文件中
            set_new_variable_to_different_range_xml_file(paste_variable_item,paste_variable_name);
            //将新建的变量，将指令插入到不同的var文件中
            set_new_variable_to_different_range_var_file(paste_variable_item);
            //删除不同被剪切变量作用域的xml文件信息
            copy_item->setText(paste_variable_name);
            delete_different_range_variable_xml(copy_item);//删除选中变量在对应xml文件的信息
            //删除其他item中的和copy_item重名的item
            for(int i=0;i<copy_item->parent()->rowCount();i++){
                if(copy_item->parent()->child(i,0)->index().data().toString()==copy_item->text()){
                    QString delete_name=copy_item->parent()->text()+copy_item->text();
                    QMap<QString,QString>::Iterator it;
                    QMap<QString,QString>::Iterator it2;
                    it=type_range_variable.find(delete_name);
                    it2=type_attribute_variable.find(delete_name);
                    type_range_variable.erase(it);
                    type_attribute_variable.erase(it2);
                    copy_item->parent()->removeRow(i);
                }
            }
        }
        else{
            //获取copy_item的数据类型
            QString search_name=copy_item->parent()->text()+copy_item->text();
            QString copy_item_type=type_attribute_variable[search_name];//数据类型

            //获取当前想插入的item
            QModelIndex current_index=ui->treeView_variable->currentIndex();
            QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
            //获取粘贴的变量名
            QString paste_variable_name=copy_item->text();
            QStandardItem* paste_variable_item=new QStandardItem(paste_variable_name);
            //将复制得到的item添加到现在的current_item上面
            current_item->appendRow(paste_variable_item);
            for(int i=0;i<copy_item->rowCount();i++){
                QString parameter_name=copy_item->child(i,0)->text();
                QStandardItem* name_item=new QStandardItem(parameter_name);
                if(copy_item->child(i,1)!=0){
                    QString parameter_value=copy_item->child(i,1)->text();
                    QStandardItem* value_item=new QStandardItem(parameter_value);
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
                else{
                    QStandardItem* value_item=new QStandardItem("0");
                    paste_variable_item->appendRow(name_item);
                    paste_variable_item->setChild(i,1,value_item);
                }
            }
            //插入copy_item的数据类型
            QString insert_name=current_item->text()+paste_variable_name;
            type_attribute_variable.insert(insert_name,copy_item_type);
            //插入copy_item的
            if(current_item->text().left(2)=="全局"){
                type_range_variable.insert(insert_name,"全局");
            }
            if(current_item->text().left(2)=="项目"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            if(current_item->text().left(2)=="程序"){
                QString temp=current_item->text().right(current_item->text().length()-2);
                type_range_variable.insert(insert_name,temp);
            }
            //将该变量添加到相应的xml文件中
            set_new_variable_to_different_range_xml_file(paste_variable_item,paste_variable_name);
            //将该变量添加到相应的var文件中
            set_new_variable_to_different_range_var_file(paste_variable_item);
        }
    }
}

//变量管理模块:剪切变量
void Widget::slt_variable_cut_variable(){
    //获取要剪切的item
    QModelIndex current_index=ui->treeView_variable->currentIndex();
    QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
    QStandardItem* item_parent=current_item->parent();
    QString delete_name=current_item->text();
    //将cut_item复制给copy_item
    copy_item=current_item;
    copy_item->setText(current_item->text()+"cut");
    qDebug()<<current_item->text();
}
//变量管理模块:重命名变量
void Widget::slt_variable_rename_variable(){
    bool isOK;
    //获取当前选中条目的父节点和程序文件名称
    QModelIndex current_index=ui->treeView_variable->currentIndex();
    QStandardItem* current_item=treemodel_variable->itemFromIndex(current_index);
    QStandardItem* parent_item=current_item->parent();
    //创建一个临时对话框，输入程序名称并且获取用户输入的程序名称
    QString old_name=current_item->text();
    QString program_text=QInputDialog::getText(NULL,"Input Dialog","请输入重命名",QLineEdit::Normal,"重命名",&isOK);
    current_item->setText(program_text);//修改树形视图的item
    //修改该变量名在xml文件中的位置
    QString insert_name=parent_item->text()+program_text;
    QString old_insert_name=parent_item->text()+old_name;
    //给修改的变量名添加数据类型和作用域
    QString insert_type=type_attribute_variable[old_insert_name];
    QString insert_range=type_range_variable[old_insert_name];
    type_range_variable.insert(insert_name,insert_range);
    type_attribute_variable.insert(insert_name,insert_type);
    //删除原来变量名的数据类型和作用域
    QMap<QString,QString>::Iterator it;
    QMap<QString,QString>::Iterator it2;
    it=type_range_variable.find(old_insert_name);
    it2=type_attribute_variable.find(old_insert_name);
    type_range_variable.erase(it);
    type_attribute_variable.erase(it2);

    if(ui->comboBox_variable_type->currentText()=="<全部>"){
        //更新节点信息进入xml文件中
        generate_all_range_variable_xml_file();//全局
        generate_project_range_variable_xml_file();//项目
        generate_program_range_variable_xml_file();//程序
    }
    else{
        //更新节点信息进入xml文件中
        rename_different_range_variable_to_xml_file(current_item,old_name,program_text);

    }

    //修改该变量名在var文件中的位置
    //根据不同的作用域，对变量var文件进行处理
    if(parent_item->text().left(2)=="全局"){
        //获取各个项目的路径
        QDir dir1(root_var_file);
        dir1.setFilter(QDir::Dirs);//顾虑配置，接受文件
        QFileInfoList list1=dir1.entryInfoList();
        for(int i=0;i<list1.count();i++){
            QString filename=list1.at(i).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString program_path=root_var_file+"//"+name;
                QDir dir2(program_path);
                dir2.setFilter(QDir::Files);
                QFileInfoList list2=dir2.entryInfoList();
                for(int l=0;l<list2.count();l++){
                    QString file_name=list2.at(l).fileName();
                    //获取文件的写入路径
                    QString path=root_var_file+"//"+name+"//"+file_name;
                    //从当前程序中对指令进行重命名
                    rename_current_choose_variable_var(path,old_name,program_text);
                }
            }
        }
    }
    if(parent_item->text().left(2)=="项目"){
        //获取项目下各个程序的路径
        QString project_path=parent_item->text().right(parent_item->text().length()-2);
        QString path1=root_var_file+"//"+project_path;
        QDir dir1(path1);
        dir1.setFilter(QDir::Files);
        QFileInfoList list1=dir1.entryInfoList();
        for(int k=0;k<list1.count();k++){
            QString filename=list1.at(k).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString path=root_var_file+"//"+project_path+"//"+name;
                //从当前程序中对指令进行重命名
                rename_current_choose_variable_var(path,old_name,program_text);
            }
        }

    }
    if(parent_item->text().left(2)=="程序"){
        //获取程序路径
        QString temp=parent_item->text().right(parent_item->text().length()-2);
        QString p2=temp.left(temp.indexOf("[",1));
        QString temp1=temp.right(temp.length()-p2.length());
        QString temp2=temp1.left(temp1.length()-1);
        QString p1=temp2.right(temp2.length()-1);
        QString path=root_var_file+"//"+p1+"//"+p2+".var";
        //从当前程序中对指令进行重命名
        rename_current_choose_variable_var(path,old_name,program_text);

    }

}
//更新节点信息进入xml文件中
void Widget::rename_different_range_variable_to_xml_file(QStandardItem *item, QString old_name, QString new_name){
    //首先判断作用域的类型
    QStandardItem* item_parent=item->parent();
    if(item_parent->text().left(2)=="全局"){
        QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    if(node_name->Value()==old_name){
                        node_name->SetValue(new_name.toStdString().c_str());
                    }
                }
            }
        }
        doc.SaveFile();
    }
    if(item_parent->text().left(2)=="项目"){
        //遍历所有项目变量的xml文件
        QString project_name=item_parent->text().right(item_parent->text().length()-2);
        QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    if(node_name->Value()==old_name){
                        node_name->SetValue(new_name.toStdString().c_str());
                    }
                }
            }
        }
        doc.SaveFile();
    }
    if(item_parent->text().left(2)=="程序"){
        //遍历所有的项目变量xml文件
        //获取项目路径的路径
        QString temp=item_parent->text().right(item_parent->text().length()-2);
        QString p2=temp.left(temp.indexOf("[",1));
        QString temp1=temp.right(temp.length()-p2.length());
        QString temp2=temp1.left(temp1.length()-1);
        QString p1=temp2.right(temp2.length()-1);
        QString path=root_xml_file+"//"+p1+"//"+p2+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
                    TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
                    TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
                    TiXmlAttribute* node_range=node_type->Next();//节点的作用域
                    if(node_name->Value()==old_name){
                        node_name->SetValue(new_name.toStdString().c_str());
                    }
                }
            }
        }
        doc.SaveFile();
    }
}
//前往相应程序文件，将相应的指令重命名
void Widget::rename_current_choose_variable_var(QString path,QString old_name,QString new_name){
    QFile file(path);//获取当前程序的路径
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    //首先读取当前文件的内容
    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("UTF-8"));
    QStringList list;
    while(!in.atEnd()){
        QString line=in.readLine();
        list.append(line);
    }
    file.close();

    //开始索引当前容器中同名的变量位置
    for(int i=1;i<list.size()-1;i++){
        QString temp=list[i].left(list[i].indexOf(":",1));
        if(old_name==temp){
            //找到了重名了
            QString suffix=list[i].right(list[i].length()-list[i].indexOf(":",1));
            QString new_value=new_name+suffix;
            int a=list.indexOf(list[i],1);
            list.replace(a,new_value);
        }
    }
    //将stringlist中的值写入文本中
    QFile file2(path);
    file2.open(QFile::WriteOnly|QFile::Truncate);
    QTextStream out(&file2);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    for(int i=0;i<list.size();i++){
        out<<list[i];
        out<<"\n";
    }

}
//前往相应var程序文件，将相应的指令删除
void Widget::delete_current_choose_variable_var(QString path, QString delete_name){
    QFile file(path);//获取当前程序的路径
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    //首先读取当前文件的内容
    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("UTF-8"));
    QStringList list;
    while(!in.atEnd()){
        QString line=in.readLine();
        list.append(line);
    }
    file.close();

    //开始索引当前容器中同名的变量位置
    for(int i=1;i<list.size()-1;i++){
        QString temp=list[i].left(list[i].indexOf(":",1));
        if(delete_name==temp){
            //存在重名的，获取当前的位置
            int a=list.indexOf(list[i],1);
            list.removeAt(a);
        }
    }
    //将stringlist中的值写入文本中
    QFile file2(path);
    file2.open(QFile::WriteOnly|QFile::Truncate);
    QTextStream out(&file2);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    for(int i=0;i<list.size();i++){
        out<<list[i];
        out<<"\n";
    }

}
//前往对应的xml程序文件，将对应的指令删除
void Widget::delete_current_choose_variable_xml(QString path,QString delete_name){
//    //打开文件
    qDebug()<<"删除变量"<<delete_name;
//    QFile file(path);
//    if(!file.open(QFile::ReadOnly)){
//        qDebug()<<"打开文件失败";
//    }
//    //删除一个一级节点及其元素，外层节点删除内层节点于此相同
//    QDomDocument doc;
//    if(!doc.setContent(&file)){
//        file.close();
//        qDebug()<<"读取文件失败";
//    }
//    file.close();//关闭文件，否则将无法操作
//    QDomElement root=doc.documentElement();
//    QDomNodeList list=doc.elementsByTagName("变量节点");//变量节点
//    for(int i=0;i<list.count();i++){
//        QDomElement e=list.at(i).toElement();
//        if(e.attribute("Name")==delete_name){
//            root.removeChild(list.at(i));

//        }
//    }
//    if(!file.open(QFile::WriteOnly|QFile::Truncate)){
//        qDebug()<<"文件重写失败";
//    }
//    //输出到文件
//    QTextStream out(&file);
//    doc.save(out,4);//缩进4格
//    file.close();
}
//变量管理模块：清空未用变量的槽函数
void Widget::on_btn_variable_clear_clicked()
{
    //首先读取变量管理模块中目前有的所有变量名
    for(int i=0;i<root_variable->rowCount();i++){
        QStandardItem* item_range=root_variable->child(i,0);//全局、项目和程序item
        for(int j=0;j<item_range->rowCount();j++){
            QStandardItem* item_variable=item_range->child(j,0);
            QString variable_name=item_variable->text();
            //判断当前变量属于什么范围的变量，采取不同的措施
            if(item_range->text().left(2)=="全局"){
                qDebug()<<"该变量属于全局变量";
            }
            else if(item_range->text().left(2)=="项目"){
                qDebug()<<"该变量属于项目变量";
            }
            else{
                //获取程序路径
                QString temp=item_range->text().right(item_range->text().length()-2);
                QString p2=temp.left(temp.indexOf("[",1));
                QString temp1=temp.right(temp.length()-p2.length());
                QString temp2=temp1.left(temp1.length()-1);
                QString p1=temp2.right(temp2.length()-1);
                QString path_prog=root_prog_file+"//"+p1+"//"+p2+".prog";
                QString path_var=root_var_file+"//"+p1+"//"+p2+".var";
                QString path_xml=root_xml_file+"//"+p1+"//"+p2+".xml";
                bool judge=flag_judge_variable_is_used_or_not(path_prog,variable_name);
                if(!judge){
                    //证明该变量没被用过，该变量可以被删除
                    delete_current_choose_variable_var(path_var,variable_name);
                    delete_current_choose_variable_xml(path_xml,variable_name);
                    //重新解析节点信息到模型中
                    if(item_variable->text()==variable_name){
                        QString temp=item_range->text()+item_variable->text();
                        QMap<QString,QString>::Iterator it;
                        QMap<QString,QString>::Iterator it2;
                        it=type_range_variable.find(temp);
                        it2=type_attribute_variable.find(temp);
                        type_range_variable.erase(it);
                        type_attribute_variable.erase(it2);
                        qDebug()<<type_range_variable.count();
                        qDebug()<<type_attribute_variable.count();
                    }
//                    //删除完xml节点信息之后，重新读取xml文件得信息，解析到模型当中
//                    int a=ui->comboBox_variable_type->currentIndex();
//                    update_xml_determined_to_variable_type(a);

                }

            }
        }
    }
}
//根据变量类型得，重新读取xml文件中得信息并重新解析到xml文件中
void Widget::update_xml_determined_to_variable_type(int a){
    if(a==0){
        //全部变量类型
        //解析全局变量作用域
        set_all_range_variable_xml_update();
        //解析程序项目变量作用域
        set_project_range_variable_xml_update();
        //解析程序变量作用域
        set_program_range_variable_xml_update();
    }
    if(a==1){
        //INTEGER
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("INTEGER");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("INTEGER");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("INTEGER");
    }
    if(a==2){
        //FLOAT
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("FLOAT");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("FLOAT");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("FLOAT");
    }
    if(a==3){
        //BOOL
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("BOOL");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("BOOL");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("BOOL");
    }
    if(a==4){
        //STRING
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("STRING");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("STRING");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("STRING");
    }
    if(a==5){
        //DYNAMIC
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("DYNAMIC");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("DYNAMIC");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("DYNAMIC");
    }
    if(a==6){
        //AXISPOS
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("AXISPOS");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("AXISPOS");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("AXISPOS");
    }
    if(a==7){
        //WORLDREFSYS
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("WORLDREFSYS");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("WORLDREFSYS");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("WORLDREFSYS");
    }
    if(a==8){
        //CARTREFSYS
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("CARTREFSYS");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("CARTREFSYS");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("CARTREFSYS");
    }
    if(a==9){
        //CARTPOS
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("CARTPOS");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("CARTPOS");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("CARTPOS");
    }
    if(a==10){
        //OVERLAP
        //解析全局变量作用域特定的数据类型
        set_detetmine_type_all_range_variable_xml_update("OVERLAP");
        //解析项目变量作用域特定的数据类型
        set_determine_type_project_range_variable_xml_update("OVERLAP");
        //解析程序变量作用已特定的数据类型
        set_detemine_type_program_range_variable_xml_update("OVERLAP");
    }
}
//判断某个变量在程序中是否用到
bool Widget::flag_judge_variable_is_used_or_not(QString path,QString name){
    bool judge=false;//判断变量是否被用到
    //首选根据文件路径，读取文件中的内容
    QFile file(path);//获取当前程序的路径
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    //首先读取当前文件的内容
    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("UTF-8"));
    QStringList list;
    while(!in.atEnd()){
        QString line=in.readLine();
        list.append(line);
    }
    file.close();
    for(int i=1;i<list.count()-1;i++){
        if(list[i].indexOf(name,1)!=-1){
            //证明该行内容中有引用到该变量
            judge=true;
            break;
        }
    }
    return judge;
}
//项目管理界面的切换
void Widget::on_btn_project_charge_clicked()
{
    int indexpage=3;
    switch_page(indexpage);
    cout<<"project_charge_widget"<<endl;

}
//项目管理模块的布局函数
void Widget::set_project_charge_widget(){
    //首先为file控件添加menu
    QMenu* file_menu=new QMenu(ui->page_project);
    ui->btn_project_file->setMenu(file_menu);
    //初始化file下的四个控件
    file_new_project=new QAction("新建项目",ui->page_project);
    file_new_program=new QAction("新建程序",ui->page_project);
    file_delete=new QAction("删除",ui->page_project);
    file_rename=new QAction("重命名",ui->page_project);
    file_copy=new QAction("复制",ui->page_project);
    file_cut=new QAction("剪切",ui->page_project);
    file_paste=new QAction("粘贴",ui->page_project);
    //将控件添加进Menu中
    file_menu->addAction(file_new_project);
    file_menu->addAction(file_new_program);
    file_menu->addAction(file_delete);
    file_menu->addAction(file_rename);
    //添加四个控件的信号槽
    connect(file_new_project,SIGNAL(triggered()),this,SLOT(slt_project_file_new_project()));//新建项目
    connect(file_new_program,SIGNAL(triggered()),this,SLOT(slt_project_file_new_program()));//新建程序
    connect(file_delete,SIGNAL(triggered()),this,SLOT(slt_project_file_delete()));//删除程序和对应的项目
    connect(file_rename,SIGNAL(triggered()),this,SLOT(slt_project_file_rename()));//对选中文件进行重命名
    //开始给试图添加条目名称
    treemodel_project=new QStandardItemModel(0,2,ui->page_project);//创建一个模型
    treemodel_project->setHeaderData(0,Qt::Horizontal,"项目");
    treemodel_project->setHeaderData(1,Qt::Horizontal,"状态");
    item_root_project=treemodel_project->invisibleRootItem();//根节点，不可见
    ui->treeView_project->setModel(treemodel_project);
    ui->treeView_project->setColumnWidth(0,300);
    ui->treeView_project->setColumnWidth(1,300);
    //首先设置项目管理模块开始的时候，界面的按钮，均不可选
    ui->btn_project_load->setEnabled(false);
    ui->btn_project_open->setEnabled(false);
    ui->btn_project_stop->setEnabled(false);
    ui->btn_project_update->setEnabled(false);
    ui->btn_project_message->setEnabled(false);
    file_new_program->setEnabled(false);
    file_rename->setEnabled(false);
    file_delete->setEnabled(false);
    //在没有程序加载的时候，程序管理模块下的编辑、宏、新建指令不可选
    ui->btn_program_edit->setEnabled(false);
    ui->btn_program_hong->setEnabled(false);
    ui->btn_program_new->setEnabled(false);
    ui->btn_program_setpc->setEnabled(false);
    ui->btn_program_edit_2->setEnabled(false);
    ui->btn_program_more->setEnabled(false);

    //当选中treeview_project时，控制项目管理模块按钮是否可以点击
    connect(ui->treeView_project->selectionModel(),&QItemSelectionModel::currentChanged,this,&Widget::slot_treeview_program_CurrentChanged);
    //解析文件路径的程序，写到treemodel_project模型上
    project_prase();
    //解析xml文件，读取上次退出时程序的状态
    parse_program_status();
}
//根据treeview_project上的状态，对程序进行相应的加载或打开操作
void Widget::base_program_status_to_operate(QString project_name,QString program_name,QString status){
    QString path=root_prog_file+"//"+project_name+"//"+program_name+".prog";//程序的路径
    if(status=="加载"){
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
            QMessageBox::warning(this,tr("Error"),tr("read file error:%1").arg(file.errorString()));
        }
        else{
            //构建QTextStream以便于读取文件
            QTextStream in(&file);
            if(file.size()!=0){
                in.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                //从stream中读取所有内容，并将其设置到QTextEdit对象（编辑框）中显示，读取过程中，设置光标为等待状态
                QApplication::setOverrideCursor(Qt::WaitCursor);//将应用程序的光标设置为等待状态
                //将读取所有文本设置到QTextEdit控件显示出来
                QStringList str;
                while (!in.atEnd()) {
                    //逐行读取文本内容的数据，并且写到QStringList中
                    QString line=in.readLine();
                    str.append(line);
                }
                for(int i=1;i<str.size()-1;i++){
                    //读取非大括号的文本内容
                    QString temp=str[i];
                    ui->program_textEdit->append(temp);
                }
                //读完文件之后，在文末插入一行END_OFF
                ui->program_textEdit->append("<<END_OFF>>");
                //读取完成后，回复光标状态
                QApplication::restoreOverrideCursor();

                //接下来设置一下光标等问题
                connect(ui->program_textEdit,SIGNAL(cursorPositionChanged()),this,SLOT(onCurrentLineHighLight()));
                connect(ui->program_textEdit,SIGNAL(cursorPositionChanged()),this,SLOT(program_text_cursorPositionChaned()));//获取光标当前选中的行数


                //切换到程序管理模块
                ui->stackedWidget->setCurrentIndex(4);
                //跳转到程序管理模块的第一个界面
                ui->stackedWidget_page_program->setCurrentIndex(0);
                //设置label_program
                QString label_program=project_name+"."+program_name;
                ui->label_program->setText(label_program);
                //设置加载按钮本身和打开按钮不可选
                ui->btn_project_load->setEnabled(false);
                ui->btn_project_open->setEnabled(false);
                //设置程序管理模块的新建按钮、编辑按钮、设置pc按钮、宏按钮等可用
                ui->btn_program_edit->setEnabled(true);
                ui->btn_program_hong->setEnabled(true);
                ui->btn_program_new->setEnabled(true);
                ui->btn_program_setpc->setEnabled(true);
                ui->btn_program_more->setEnabled(true);
                ui->btn_program_edit_2->setEnabled(true);
                //设置program_edit只读
                ui->program_textEdit->setReadOnly(true);

                //设置设置label_information
                QString information="加载"+label_program;
                ui->label_information->setText(information);
                label_information_text.push(information);

                //每次加载完，都要将存放容器清空，避免不同程序加载时重名的情况
                type_range.clear();
                type_attribute.clear();
            }


        }
    }
    if(status=="打开"){
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
            QMessageBox::warning(this,tr("Error"),tr("read file error:%1").arg(file.errorString()));
        }
        else{
            //构建QTextStream以便于读取文件
            QTextStream in(&file);
            in.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
            //从stream中读取所有内容，并将其设置到QTextEdit对象（编辑框）中显示，读取过程中，设置光标为等待状态
            QApplication::setOverrideCursor(Qt::WaitCursor);//将应用程序的光标设置为等待状态
            //将读取所有文本设置到QTextEdit控件显示出来
            QStringList str;
            while (!in.atEnd()) {
                //逐行读取文本内容的数据，并且写到QStringList中
                QString line=in.readLine();
                str.append(line);
            }
            for(int i=1;i<str.size()-1;i++){
                //读取非大括号的文本内容
                QString temp=str[i];
                ui->program_textEdit->append(temp);
            }

            //读取完成后，回复光标状态
            QApplication::restoreOverrideCursor();

            //切换到程序管理模块
            ui->stackedWidget->setCurrentIndex(4);
            //设置QTextedit的背景为灰色
            QPalette pal=ui->program_textEdit->palette();
            pal.setBrush(QPalette::Base,Qt::lightGray);
            ui->program_textEdit->setPalette(pal);
            //设置program_textedit为只读
            ui->program_textEdit->setReadOnly(true);

            //设置程序管理模块下控件都不可以点击
            ui->btn_program_edit->setEnabled(false);
            ui->btn_program_hong->setEnabled(false);
            ui->btn_program_new->setEnabled(false);
            ui->btn_program_setpc->setEnabled(false);
            ui->btn_program_edit_2->setEnabled(false);
            ui->btn_program_edit_2->setEnabled(false);
            ui->btn_program_more->setEnabled(false);
            //设置项目管理模块，加载按钮，打开按钮不可点击
            ui->btn_project_load->setEnabled(false);
            ui->btn_project_open->setEnabled(false);
            //设置label_program
            QString program=project_name+"."+program_name;
            ui->label_program->setText(program);
            //设置一下状态栏显示程序的名称
            QString information="打开"+program;
            ui->label_information->setText(information);
            label_information_text.push(information);
        }
    }
}
//解析xml文件，读取上次退出时程序的状态
void Widget::parse_program_status(){
    //创建一个xml文档
    TiXmlDocument doc;
    if(doc.LoadFile("status.xml")){
        TiXmlElement* root=doc.RootElement();
        if(root){
            //遍历treeview_project下的项目变量和程序变量
            for(int i=0;i<item_root_project->rowCount();i++){
                QString project_item_name=item_root_project->child(i,0)->text();
                for(TiXmlElement* node_project=root->FirstChildElement();node_project;node_project=node_project->NextSiblingElement()){
                    if(project_item_name==node_project->Value()){
                        //将xml中对应的项目变量的状态添加到treeview中
                        QStandardItem* project_status_item=new QStandardItem(node_project->FirstAttribute()->Value());
                        item_root_project->setChild(i,1,project_status_item);
                        //设置完项目的状态之后，开始设置treeview_item下程序的状态
                        for(int j=0;j<item_root_project->child(i,0)->rowCount();j++){
                            QString program_item_name=item_root_project->child(i,0)->child(j,0)->text();
                            for(TiXmlElement* node_program=node_project->FirstChildElement();node_program;node_program=node_program->NextSiblingElement()){
                                if(program_item_name==node_program->Value()){
                                    //将xml中对应的程序变量的状态添加到treeview中
                                    QStandardItem* program_status_item=new QStandardItem(node_program->FirstAttribute()->Value());
                                    item_root_project->child(i,0)->setChild(j,1,program_status_item);
                                    //根据treeview_project上的状态，对程序进行相应的加载或打开操作
//                                    base_program_status_to_operate(project_item_name,program_item_name,node_program->FirstAttribute()->Value());
                                    //设置或者打开的item为保持焦点状态
                                    if(program_status_item->text()=="终止"){
                                        //设置对应的item为选中状态
                                        QStandardItem* set_item=item_root_project->child(i,0)->child(j,0);
                                        QModelIndex index=set_item->index();
                                        ui->treeView_project->setCurrentIndex(index);
                                    }
                                }

                            }
                        }
                    }
                }
            }
        }
    }
}
// 选中treeview_project上的项目或者程序item时，处理控件是否能点击的槽函数
void Widget::slot_treeview_program_CurrentChanged(const QModelIndex& t, const QModelIndex& previous){
    QStandardItem* item1=treemodel_project->itemFromIndex(t);
    QStandardItem* item2=treemodel_project->itemFromIndex(previous);
    if(item1->parent()!=NULL){
        //选中的是程序，加载按钮，打开按钮，关闭按钮，删除按钮，重命名按钮可以点击
        ui->btn_project_load->setEnabled(true);
        ui->btn_project_open->setEnabled(true);
        ui->btn_project_stop->setEnabled(true);
        file_delete->setEnabled(true);
        file_rename->setEnabled(true);
    }
    else{
        //选中的是项目，只有新建程序、删除、重命名按钮可以点击
        ui->btn_project_load->setEnabled(false);
        ui->btn_project_open->setEnabled(false);
        ui->btn_project_stop->setEnabled(false);
        file_new_program->setEnabled(true);
    }

}

//项目管理模块：加载按钮槽函数
void Widget::on_btn_project_load_clicked()
{
    if(hasLoad == 1)
    {
       QMessageBox::warning(ui->page_project,"加载程序","请先终止当前的程序后再加载");
       return;
    }
    else
    {
        //获取当前选中条目的父节点的项目名称
        QModelIndex current_index=ui->treeView_project->currentIndex();//获取当前选中的条目
        QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前索引下的item
        QStandardItem* parent=current_item->parent();
        QString project_name=parent->text();//这个就是项目名称
        //获取当前加载项目的名称
        projName = project_name.toStdString();
        //获取当前选中的程序文件名称
        QString program_name=current_item->text();
        //获取当前加载程序的名称
        varProgName = program_name.toStdString();
        //获取当前加载程序的路径
        QString current_prog_path=root_prog_file+"//"+project_name+"//"+program_name+".prog";
        QFile file(current_prog_path);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
            QMessageBox::warning(this,tr("Error"),tr("read file error:%1").arg(file.errorString()));
        }
        else{
            //构建QTextStream以便于读取文件
            QTextStream in(&file);
            in.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
            //从stream中读取所有内容，并将其设置到QTextEdit对象（编辑框）中显示，读取过程中，设置光标为等待状态
            QApplication::setOverrideCursor(Qt::WaitCursor);//将应用程序的光标设置为等待状态
            //将读取所有文本设置到QTextEdit控件显示出来
            QStringList str;
            while (!in.atEnd()) {
                //逐行读取文本内容的数据，并且写到QStringList中
                QString line=in.readLine();
                str.append(line);
            }
            for(int i=1;i<str.size()-1;i++){
                //读取非大括号的文本内容
                QString temp=str[i];
                ui->program_textEdit->append(temp);
            }
            //读完文件之后，在文末插入一行END_OFF
            ui->program_textEdit->append("<<END_OFF>>");

            //读取完成后，回复光标状态
            QApplication::restoreOverrideCursor();

            //设置treeview_projet上状态的函数
            set_project_program_status("加载");
            //接下来设置一下光标等问题
            connect(ui->program_textEdit,SIGNAL(cursorPositionChanged()),this,SLOT(onCurrentLineHighLight()));
            connect(ui->program_textEdit,SIGNAL(cursorPositionChanged()),this,SLOT(program_text_cursorPositionChaned()));//获取光标当前选中的行数

            //切换到程序管理模块
            ui->stackedWidget->setCurrentIndex(4);
            //跳转到程序管理模块的第一个界面
            ui->stackedWidget_page_program->setCurrentIndex(0);
            //设置label_program
            QString label_program=project_name+"."+program_name;
            ui->label_program->setText(label_program);
            //设置加载按钮本身和打开按钮不可选
            ui->btn_project_load->setEnabled(false);
            ui->btn_project_open->setEnabled(false);
            //设置程序管理模块的新建按钮、编辑按钮、设置pc按钮、宏按钮等可用
            ui->btn_program_edit->setEnabled(true);
            ui->btn_program_hong->setEnabled(true);
            ui->btn_program_new->setEnabled(true);
            ui->btn_program_setpc->setEnabled(true);
            ui->btn_program_more->setEnabled(true);
            ui->btn_program_edit_2->setEnabled(true);
            //设置program_edit只读
            ui->program_textEdit->setReadOnly(true);

            //设置设置label_information
            QString information="加载"+label_program;
            ui->label_information->setText(information);
            label_information_text.push(information);

            //每次加载完，都要将存放容器清空，避免不同程序加载时重名的情况
            type_range.clear();
            type_attribute.clear();
            //刷新保存程序状态的xml文件
            update_program_status();
            //是否已经加载标志置为1
            hasLoad = 1;
            //程序是否激活标志置为1
            activeState = 1;
        }
    }
}
//刷新保存程序状态的xml文件
void Widget::update_program_status(){
    //创建一个xml文档
    TiXmlDocument doc;
    //添加文档头部的基本声明
    TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","UTF-8","yes");
    doc.LinkEndChild(dec);
    //为文档添加基本的注释
    TiXmlComment* comment=new TiXmlComment("This is status for program");
    doc.LinkEndChild(comment);
    //为xml文档添加根元素
    TiXmlElement* root=new TiXmlElement("root");
    //开始写数据进xml文件中
    if(item_root_project->rowCount()){
        //证明有项目变量
        vector<TiXmlElement*> node_project(item_root_project->rowCount());
        for(int i=0;i<item_root_project->rowCount();i++){
            //获取项目的名称
            QString project_name=item_root_project->child(i,0)->text();
            node_project[i]=new TiXmlElement(project_name.toStdString().c_str());
            //获取项目的状态
            if(item_root_project->child(i,1)->text()!="----"){
                node_project[i]->SetAttribute("Status","终止");
            }
            else{
                node_project[i]->SetAttribute("Status","----");
            }
            root->LinkEndChild(node_project[i]);
            //设置完项目变量之后，设置项目下的程序变量
            vector<TiXmlElement*> node_program(item_root_project->child(i,0)->rowCount());
            if(item_root_project->child(i,0)->rowCount()){
                //证明有程序变量
                for(int j=0;j<item_root_project->child(i,0)->rowCount();j++){
                    //获取程序的名称
                    QString program_name=item_root_project->child(i,0)->child(j,0)->text();
                    node_program[j]=new TiXmlElement(program_name.toStdString().c_str());
                    //获取程序的状态
                    if(item_root_project->child(i,0)->child(j,1)->text()!="----"){
                        node_program[j]->SetAttribute("Status","终止");
                    }
                    else{
                        node_program[j]->SetAttribute("Status","----");
                    }
                    node_project[i]->LinkEndChild(node_program[j]);
                }
            }
        }
    }
    doc.LinkEndChild(root);
    doc.SaveFile("status.xml");
}
//项目管理模块，打开按钮的槽函数
void Widget::on_btn_project_open_clicked()
{
    //获取当前选中条目的父节点的项目名称
    QModelIndex current_index=ui->treeView_project->currentIndex();//获取当前选中的条目
    QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前索引下的item
    QStandardItem* parent=current_item->parent();
    QString project_name=parent->text();//这个就是项目名称
    //获取当前选中的程序文件名称
    QString program_name=current_item->text();
    //获取程序绝对路径
    QString abosolute_prog=root_prog_file+"//"+project_name+"//"+program_name+".prog";
    QString abosolute_var=root_var_file+"//"+project_name+"//"+program_name+".var";
    QString abosolute_xml=root_xml_file+"//"+project_name+"//"+program_name+".xml";

    QFile file(abosolute_prog);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,tr("Error"),tr("read file error:%1").arg(file.errorString()));
    }
    else{
        //构建QTextStream以便于读取文件
        QTextStream in(&file);
        in.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
        //从stream中读取所有内容，并将其设置到QTextEdit对象（编辑框）中显示，读取过程中，设置光标为等待状态
        QApplication::setOverrideCursor(Qt::WaitCursor);//将应用程序的光标设置为等待状态
        //将读取所有文本设置到QTextEdit控件显示出来
        QStringList str;
        while (!in.atEnd()) {
            //逐行读取文本内容的数据，并且写到QStringList中
            QString line=in.readLine();
            str.append(line);
        }
        for(int i=1;i<str.size()-1;i++){
            //读取非大括号的文本内容
            QString temp=str[i];
            ui->program_textEdit->append(temp);
        }

        //读取完成后，回复光标状态
        QApplication::restoreOverrideCursor();

        //设置treeview_projet上状态的函数
        set_project_program_status("打开");

        //生成一xml文件，包含当前项目、程序名称以及相应状态的xml文件

        //切换到程序管理模块
        ui->stackedWidget->setCurrentIndex(4);
        //设置QTextedit的背景为灰色
        QPalette pal=ui->program_textEdit->palette();
        pal.setBrush(QPalette::Base,Qt::lightGray);
        ui->program_textEdit->setPalette(pal);
        //设置program_textedit为只读
        ui->program_textEdit->setReadOnly(true);

        //设置程序管理模块下控件都不可以点击
        ui->btn_program_edit->setEnabled(false);
        ui->btn_program_hong->setEnabled(false);
        ui->btn_program_new->setEnabled(false);
        ui->btn_program_setpc->setEnabled(false);
        ui->btn_program_edit_2->setEnabled(false);
        ui->btn_program_edit_2->setEnabled(false);
        ui->btn_program_more->setEnabled(false);
        //设置项目管理模块，加载按钮，打开按钮不可点击
        ui->btn_project_load->setEnabled(false);
        ui->btn_project_open->setEnabled(false);
        //设置label_program
        QString program=project_name+"."+program_name;
        ui->label_program->setText(program);
        //设置一下状态栏显示程序的名称
        QString information="打开"+program;
        ui->label_information->setText(information);
        label_information_text.push(information);
        //刷新保存程序状态的xml文件
        update_program_status();
    }
}
//项目管理模块：终止按钮槽函数
void Widget::on_btn_project_stop_clicked()
{
    //如何有程序正在运行中，提示要停止运行后才能终止程序
    if (runState == 1)
    {
        QMessageBox::warning(this,"终止程序或项目","请先停止运行当前程序后再终止程序或项目");
        return;
    }
    //清空程序管理界面的文本内容
    ui->program_textEdit->clear();
    ui->program_textEdit->setReadOnly(true);//设置只读
    //设置treeview_project的显示状态
    set_project_program_status("终止");
    //设置状态栏显示的名称

    //设置程序管理模块下控件都不可以点击
    ui->btn_program_edit->setEnabled(false);
    ui->btn_program_hong->setEnabled(false);
    ui->btn_program_new->setEnabled(false);
    ui->btn_program_setpc->setEnabled(false);
    ui->btn_program_edit_2->setEnabled(false);
    ui->btn_program_edit_2->setEnabled(false);
    ui->btn_program_more->setEnabled(false);
    //设置QTextedit的背景为灰色
    QPalette pal=ui->program_textEdit->palette();
    pal.setBrush(QPalette::Base,Qt::white);
    ui->program_textEdit->setPalette(pal);
    //设置加载、打开按钮可选
    ui->btn_project_load->setEnabled(true);
    ui->btn_project_open->setEnabled(true);
    //清除label_program
    ui->label_program->clear();

    //设置一下label_information
    //获取当前选中条目的父节点的项目名称
    QModelIndex current_index=ui->treeView_project->currentIndex();//获取当前选中的条目
    QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前索引下的item
    QStandardItem* parent=current_item->parent();
    QString project_name=parent->text();//这个就是项目名称
    //获取当前选中的程序文件名称
    QString program_name=current_item->text();
    QString program=project_name+"."+program_name;

    QString information="终止"+program;
    ui->label_information->setText(information);
    label_information_text.push(information);
    //刷新保存程序状态的xml文件
    update_program_status();
    //是否已经加载标志置为0
    hasLoad = 0;
    //程序是否激活标志置为0
    activeState = 0;
}
//通过该函数使光标所在行高亮
void Widget::onCurrentLineHighLight(){
    QList<QTextEdit::ExtraSelection>extraSelection;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection,true);
    selection.cursor = ui->program_textEdit->textCursor();
    selection.cursor.clearSelection();
    extraSelection.append(selection);
    ui->program_textEdit->setExtraSelections(extraSelection);
}
//获取文本中光标所在行数的槽函数
int Widget::program_text_cursorPositionChaned(){
    //当前光标
    QTextCursor cur=ui->program_textEdit->textCursor();
    QTextLayout* layout=cur.block().layout();
    //当前光标在本Block内的相对位置
    int nCurpos=cur.position()-cur.block().position();
    int ntextline=layout->lineForTextPosition(nCurpos).lineNumber()+cur.block().firstLineNumber();
    line_program_edit=ntextline;
    return ntextline;
}

//设置treeview_project项目和程序的加载、打开以及终止状态的槽函数
void Widget::set_project_program_status(QString a){
    //获取当前选中条目的父节点的项目名称
    QModelIndex current_index=ui->treeView_project->currentIndex();//获取当前选中的条目
    QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前索引下的item
    QStandardItem* parent=current_item->parent();
    //需要设计算法，求出当前索引项目的行数
    int count_program=0;
    int count_project=0;
    //计算出当前程序的行数
    for(int i=0;i<parent->rowCount();i++){
        //设置所选程序和对应项目的status为loaded
        QStandardItem* status=new QStandardItem(a);
        if(current_index.data().toString()==parent->child(i)->text()){
            parent->setChild(i,1,status);//测试成功
        }
        //将当前项目下的其它程序文件的text设置为空
        else {
            QStandardItem* item_status=new QStandardItem("----");
            parent->setChild(i,1,item_status);
        }
    }
    //计算当前程序所在项目的行数

    for(int i=0;i<item_root_project->rowCount();i++){
        //设置所选程序和对应项目的status为loaded
        QStandardItem* status=new QStandardItem(a);
        QModelIndex index=treemodel_project->index(i,0,QModelIndex());
        if(current_index.parent().data().toString()==treemodel_project->data(index,Qt::EditRole).toString()){
            treemodel_project->setItem(i,1,status);
        }
        else{
            //其它项目均设置为----状态
            QStandardItem* item_status=new QStandardItem("----");
            treemodel_project->setItem(i,1,item_status);
            QStandardItem* other_item_project=treemodel_project->item(i);//获取其余项目的名称
            for(int j=0;j<other_item_project->rowCount();j++){
                other_item_project->setChild(j,1,new QStandardItem("----"));
            }

        }
    }

}
//项目管理模块，新建项目的槽函数
void Widget::slt_project_file_new_project(){
    bool isOK;//该布尔值作判断
    //创建一临时对话框，输入项目名称并且获取用户输入的项目名称
    QString text=QInputDialog::getText(NULL,"Input Dialog","请输入项目名称",QLineEdit::Normal,"项目名称",&isOK);
    if(isOK){
        //新建了project之后，需要在电脑某路径之下创建该文件名的项目文件夹
        QString dir_name=text;//将从dialog中读取到的项目名称赋值给filename
        QString file_path_name=root_prog_file+"//"+dir_name;//项目路径加上文件夹名称
        QString file_var_path_name=root_var_file+"//"+dir_name;//相应var项目文件加上文件夹名称
        QDir* dic=new QDir();
        //创建prog文件夹
        bool exist=dic->exists(file_path_name);
        if(exist){
            QMessageBox::warning(ui->page_project,"创建文件夹","prog文件夹已存在");
        }
        else{
            dic->mkdir(file_path_name);//在指定的目录下创建用户输入的文件夹名称
            //将用户输入的项目名称作为Item显示到treeview上
            QStandardItem* project_item=new QStandardItem(text);
            item_root_project->appendRow(project_item);
        }
        //创建相应的var文件夹项目
        bool exist_var=dic->exists(file_var_path_name);
        if(exist_var){
            QMessageBox::warning(ui->page_project,"创建文件夹","var文件夹已存在");
        }
        else{
            dic->mkdir(file_var_path_name);//在指定目录下创建用户输入的文件夹名称
        }
        //创建相应的xml文件夹项目
        QString file_xml_path_name=root_xml_file+"//"+dir_name;
        bool exist_xml=dic->exists(file_xml_path_name);
        if(exist_xml){
            QMessageBox::warning(ui->page_project,"创建文件夹","xml文件夹已存在");
        }
        else{
            dic->mkdir(file_xml_path_name);//创建xml文件
        }
    }
}
//项目管理模块，新建程序的槽函数
void Widget::slt_project_file_new_program(){
    bool isOK;//创建一个布尔值为后面作判断
    //创建一个临时对话框，输入程序名称并且获取用户输入的程序名称
    QString program_text=QInputDialog::getText(NULL,"Input Dialog","请输入程序名称",QLineEdit::Normal,"程序名称",&isOK);
    if(isOK){
        QString dir_name=ui->treeView_project->currentIndex().data().toString();//读取目前选中的项目名称
        QString file_name=program_text;//获取当前用户输入的程序名称
        QString file_path=root_prog_file+"//"+dir_name+"//"+file_name+".prog";//prog文件的绝对路径

        QString file_path_var=root_var_file+"//"+dir_name+"//"+file_name+".var";//var文件的绝对路径
        QString file_path_xml=root_xml_file+"//"+dir_name+"//"+file_name+".xml";//xml文件的绝对路径


        //首先创建prog程序文件
        QFile file(file_path);
        if(file.exists()){
            QMessageBox::warning(ui->page_project,"创建程序","程序已存在");
        }
        else{
            //没有这个程序，开始创建
            file.open(QIODevice::ReadWrite|QIODevice::Text);
            file.close();
            //获取鼠标选中的当前索引
            QModelIndex temp=ui->treeView_project->currentIndex();
            QStandardItem* current=treemodel_project->itemFromIndex(temp);//获取当前索引下的item
            QStandardItem* program=new QStandardItem(program_text);//将用户输入的程序名称复制给程序item
            current->appendRow(program);//将程序添加到选中的项目item下
        }
        //接着创建var文件
        QFile file_var(file_path_var);
        if(file_var.exists()){
            QMessageBox::warning(ui->page_project,"创建var文件","var文件已存在");
        }
        else{
            //没有这个var文件，开始创建
            file_var.open(QIODevice::ReadWrite|QIODevice::Text);
            file_var.close();
            //所创建的新的程序文件，将全局var和该程序对应的项目var文件中的变量信息写到该程序var文件中
            set_all_project_range_to_new_var_file(dir_name,file_name);
        }
        //接着创建xml文件
        QFile file_xml(file_path_xml);
        if(file_xml.exists()){
            QMessageBox::warning(ui->page_project,"创建xml文件","xml文件已存在");
        }
        else{
            //没有这个xml文件，开始创建
            file_xml.open(QIODevice::ReadWrite|QIODevice::Text);
            file_xml.close();
        }
        //最后，展开所有节点
        ui->treeView_project->expandAll();
    }

}
//所创建的新的程序文件，将全局var和该程序对应的项目var文件中的变量信息写到该程序var文件中
void Widget::set_all_project_range_to_new_var_file(QString project_path,QString program_path){
    //解析全局变量作用域
    set_all_range_variable_xml_update();
    //解析程序项目变量作用域
    set_project_range_variable_xml_update();
    //解析程序变量作用域
    set_program_range_variable_xml_update();
    //设置变量类型下拉框显示为全部
    ui->comboBox_variable_type->setCurrentIndex(0);
    //首先判断项目和全局下是否有变量
    for(int i=0;i<root_variable->rowCount();i++){
        if(root_variable->child(i,0)->text().left(2)=="全局"){
            //全局变量
            QStandardItem* all_item=root_variable->child(i,0);
            if(all_item->rowCount()){
                //存在全局变量，获取全局变量的数据类型
                for(int j=0;j<all_item->rowCount();j++){
                    QStandardItem* all_variable_item=all_item->child(j,0);
                    QString judge_name=all_item->text()+all_variable_item->text();
                    QString type=type_attribute_variable[judge_name];
                    //获取对应变量类型的数据类型
                    QString insert_format=get_different_format_var_string(all_variable_item,type);
                    //获取程序的路径
                    QString path=root_var_file+"//"+project_path+"//"+program_path+".var";
                    qDebug()<<path;
                    //将指令插入到当前的var项目中
                    insert_format_value_to_var_file(path,insert_format);
                }
            }
        }
        if(root_variable->child(i,0)->text().left(2)=="项目"){
            //项目变量
            QStandardItem* project_item=root_variable->child(i,0);
            if(project_item->rowCount()){
                qDebug()<<"存在项目变量";
                //存在项目变量，获取项目变量的数据类型
                for(int j=0;j<project_item->rowCount();j++){
                    QStandardItem* project_variable_item=project_item->child(j,0);
                    QString judge_name=project_item->text()+project_variable_item->text();
                    QString type=type_attribute_variable[judge_name];
                    //获取对应的数据类型
                    QString insert_format=get_different_format_var_string(project_variable_item,type);
                    //获取程序的路径
                    QString path=root_var_file+"//"+project_path+"//"+program_path+".var";
                    //将指令插入到当前的var项目中
                    insert_format_value_to_var_file(path,insert_format);
                }
            }
        }
    }
}
//项目管理模块，删除程序或者项目的槽函数
void Widget::slt_project_file_delete(){
    QModelIndex current_index=ui->treeView_project->currentIndex();//获取当前选中的索引
    QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前索引下的item
    //判断当前所选的item是项目节点还是程序节点
    if(current_item->parent()==0){
//        //删项目
        QString dir_name=current_index.data().toString();
        QString file_path_name=root_prog_file+"//"+dir_name;//获取根目录加加上当前文件夹名称所组成的目录
        QString file_var_path_name=root_var_file+"//"+dir_name;//获取根据加上var文件夹所组成的目录路径
        QString file_xml_path_name=root_xml_file+"//"+dir_name;//xml根目录加上
        QDir dir(file_path_name);
        dir.removeRecursively();//删除prog的项目
        QDir dir_var(file_var_path_name);
        dir_var.removeRecursively();//删除var的项目
        QDir dir_xml(file_xml_path_name);
        dir_xml.removeRecursively();//删除xml的项目
        //删除当前视图上的所对应的项目
        if(item_root_project->rowCount()==1){
            treemodel_project->clear();
            treemodel_project=new QStandardItemModel(0,2,ui->page_project);//创建一个模型
            treemodel_project->setHeaderData(0,Qt::Horizontal,"项目");
            treemodel_project->setHeaderData(1,Qt::Horizontal,"状态");
            item_root_project=treemodel_project->invisibleRootItem();//根节点，不可见
            ui->treeView_project->setModel(treemodel_project);
            ui->treeView_project->setColumnWidth(0,300);
            ui->treeView_project->setColumnWidth(1,300);
            //因为这个删除的是最后当前项目管理模块中唯一的一个项目，所以可以在删除的同时，删除all文件
            QString all_xml_path=root_xml_file+"//"+"all.xml";
            QFile all_xml_file(all_xml_path);
            all_xml_file.remove();
        }
        else{
            //当前只有一个项目时，通过这种方式删除
            for(int i=0;i<item_root_project->rowCount();i++){
                if(item_root_project->child(i,0)->index().data().toString()==current_index.data().toString()){
                    item_root_project->removeRow(i);
                }
            }
        }
    }
    else{
        QStandardItem* parent=current_item->parent();
        //删项目下的程序
        QString dir_name=parent->text();//当前程序的项目名称
        QString file_name=current_index.data().toString();//获取当前选中的文件名称
        QString file_path=root_prog_file+"//"+dir_name+"//"+file_name+".prog";//当前文件的绝对路径
        QFile file(file_path);
        file.remove();//删除当前所选的prog程序文件

        //删除var项目下的var文件
        QString file_path_var=root_var_file+"//"+dir_name+"//"+file_name+".var";
        QFile file_var(file_path_var);
        file_var.remove();//删除对应的var程序文件

        //删除xml项目下的xml文件
        QString file_path_xml=root_xml_file+"//"+dir_name+"//"+file_name+".xml";//对应程序的ptp文件
        QFile file_xml(file_path_xml);
        file_xml.remove();

        //删除界面上treeview_project上对应的文件名
        for(int i=0;i<parent->rowCount();i++){
            if(parent->child(i)->text()==current_index.data().toString()){
                parent->removeRow(i);
            }
        }
    }
}
//项目管理模块，对文件进行重命名
void Widget::slt_project_file_rename(){
    bool isOK;
    //获取当前选中条目的父节点的项目名称
    QModelIndex current_index=ui->treeView_project->currentIndex();//获取当前选中的条目
    QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前索引下的item
    //若child==0，则此时更改的程序文件名称
    if(current_item->child(0)==0){
        QString old_name=ui->treeView_project->currentIndex().data().toString();
        //创建一个临时对话框，输入程序名称并且获取用户输入的程序名称
        QString program_text=QInputDialog::getText(NULL,"Input Dialog","请输入重命名",QLineEdit::Normal,"重命名",&isOK);
        //读取文件版本1
        QStandardItem* parent=current_item->parent();
        QString project_name=parent->text();//这个就是项目名称
        //获取当前选中的程序文件名称
        QString program_name=program_text;

        if(isOK){
            //修改prog程序文件
            QString absolute_prog=root_prog_file+"//"+project_name+"//"+program_name+".prog";//获取新的prog程序绝对路径
            QString old_absolute_prog=root_prog_file+"//"+project_name+"//"+old_name+".prog";//获取旧的prog程序绝对路径
            QFile file(absolute_prog);
            if(file.exists()){
                QMessageBox::warning(ui->page_project,"更改文件名","文件已存在");
            }
            else{
                current_item->setText(program_text);//修改选中的名称
                //没有这个名称的文件，开始更改
                QFile::rename(old_absolute_prog,absolute_prog);
            }
            //修改var程序文件名
            QString abosolute_var=root_var_file+"//"+project_name+"//"+program_name+".var";//获取新的var程序绝对路径
            QString old_abosolue_var=root_var_file+"//"+project_name+"//"+old_name+".var";//获取旧的var程序绝对路径
            QFile file_var(abosolute_var);
            if(file_var.exists()){
                QMessageBox::warning(ui->page_project,"更改var文件名","var文件已存在");
            }
            else{
                QFile::rename(old_abosolue_var,abosolute_var);
            }
            //修改xml程序文件名
            QString abosolue_xml=root_xml_file+"//"+project_name+"//"+program_name+".xml";

            QString old_abosolute_xml=root_xml_file+"//"+project_name+"//"+old_name+".xml";

            QFile file_PTP_xml(abosolue_xml);
            if(file_PTP_xml.exists()){
                QMessageBox::warning(ui->page_project,"更改xml文件名","xml文件已存在");
            }
            else{
                QFile::rename(old_abosolute_xml,abosolue_xml);
            }
        }
    }
    //若child!=0,则此时更改的是项目的名称
    else{
        //获取原来的项目名称
        QString old_name=ui->treeView_project->currentIndex().data().toString();
        //创建一个临时对话框，输入程序名称并且获取用户输入的程序名称
        QString program_text=QInputDialog::getText(NULL,"Input Dialog","please ReName File_Name",QLineEdit::Normal,"File name",&isOK);
        QString new_name=program_text;//修改后的项目名称
        QString old_project_path=root_prog_file+"//"+old_name;//原prog项目路径
        QString new_project_path=root_prog_file+"//"+new_name;//新prog项目路径
        QString old_project_var_path=root_var_file+"//"+old_name;//原var项目路径
        QString new_project_var_path=root_var_file+"//"+new_name;//新var项目路径
        QString old_project_xml_path=root_xml_file+"//"+old_name;//原xml项目路径
        QString new_project_xml_path=root_xml_file+"//"+new_name;//新xml项目路径



        if(isOK){
            QDir dir_prog(new_project_path);
            if(dir_prog.exists()){
                QMessageBox::warning(ui->page_project,"更改项目名","prog项目已存在");
            }
            else{
                if(dir_prog.rename(old_project_path,new_project_path)){
                    qDebug()<<"重命名成功";
                }
                else{
                    qDebug()<<"重命名失败";
                }

            }
            QDir dir_var(new_project_var_path);
            if(dir_var.exists()){
                QMessageBox::warning(ui->page_project,"更改项目名","var项目已存在");
            }
            else{
                if(dir_var.rename(old_project_var_path,new_project_var_path)){
                    qDebug()<<"重命名成功";
                }
                else{
                    qDebug()<<"重命名失败";
                }
            }
            QDir dir_xml(new_project_xml_path);
            if(dir_xml.exists()){
                QMessageBox::warning(ui->page_project,"更改项目名","xml项目已存在");
            }
            else{
                if(dir_xml.rename(old_project_xml_path,new_project_xml_path)){
                    qDebug()<<"重命名成功";
                }
                else{
                    qDebug()<<"重命名失败";
                }
            }
            QModelIndex index=ui->treeView_project->currentIndex();//获取当前item索引
            QStandardItem* current_item=treemodel_project->itemFromIndex(index);//获取当前item
            current_item->setText(program_text);//修改树形控件中的文件名称
        }
    }
}
//项目管理模块：刷新按钮的槽函数
void Widget::on_btn_project_update_clicked()
{
    //开始给视图添加条目名称
    treemodel_project=new QStandardItemModel(0,2,ui->page_project);//创建一个模型
    treemodel_project->setHeaderData(0,Qt::Horizontal,"项目");
    treemodel_project->setHeaderData(1,Qt::Horizontal,"状态");
    item_root_project=treemodel_project->invisibleRootItem();//根节点，不可见
    ui->treeView_project->setModel(treemodel_project);
    ui->treeView_project->setColumnWidth(0,300);
    ui->treeView_project->setColumnWidth(1,300);
    project_prase2();
}
//project_parse()槽函数：进入项目管理模块，解析路径文件显示到界面上
void Widget::project_prase(){
    //首先创建根目录，prog目录，var目录，xml目录的文件夹
    QDir* dir_root=new QDir();
    QDir* dir_root_prog=new QDir();
    QDir* dir_root_var=new QDir();
    QDir* dir_root_xml=new QDir();
    //若根目录不存在，创建根目录
    bool exist_root=dir_root->exists(root_path);
    if(!exist_root){
        //设置新的目录来创建文件
        bool OK=dir_root->mkdir(root_path);
        if(OK){
            qDebug()<<"创建成功";
        }
        else{
            QMessageBox::warning(ui->page_project,"创建根目录","文件夹创建失败");
        }

    }
    //若prog根目录不存在，创建prog根目录

    bool exist_root_prog=dir_root_prog->exists(root_prog_file);
    if(!exist_root_prog){
        bool OK=dir_root_prog->mkdir(root_prog_file);
        if(OK){
            qDebug()<<"创建成功";
        }
        else{
            QMessageBox::warning(ui->page_project,"创建prog目录","prog目录创建失败");
        }

    }
    //若var根目录不存在，创建var根目录
    bool exist_root_var=dir_root_var->exists(root_var_file);
    if(!exist_root_var){
        bool OK=dir_root_var->mkdir(root_var_file);
        if(OK){
            qDebug()<<"创建成功";
        }
        else{
            QMessageBox::warning(ui->page_project,"创建var目录","var目录创建失败");
        }
    }
    //若xml根目录不存在，创建xml根目录
    bool exist_root_xml=dir_root_xml->exists(root_xml_file);
    if(!exist_root_xml){
        bool OK=dir_root_xml->mkdir(root_xml_file);
        if(OK){
            qDebug()<<"创建成功";
        }
        else{
            QMessageBox::warning(ui->page_project,"创建xml目录","xml目录创建失败");
        }
    }
    //解析prog文件到视图上的函数,也是更新按钮的函数作用
    project_prase2();
}
//项目管理模块，解析prog文件显示到视图上的第二部
void Widget::project_prase2(){
    //开始解析prog根目录下的项目和程序文件写入treeview_project上
    QDir dir(root_prog_file);
    dir.setFilter(QDir::Dirs);//设置过滤配置，接受文件
    QFileInfoList list=dir.entryInfoList();
    QStringList fileList;
    for(int i=0;i<list.count();i++){
        QString filename=list.at(i).fileName();
        if(filename=="."||filename==".."){

        }
        else{
            QString name=filename;//获取非"."和".."的文件
//            qDebug()<<name;
            //插入项目节点之前需要判断，treeview中是否已经有同名的项目文件
            //如果root下无项目节点，直接插入项目，无需比较
            if(item_root_project->rowCount()==0){
                QStandardItem* item_project=new QStandardItem(name);//插入项目节点
                item_root_project->appendRow(item_project);//将项目名称添加进节点中
                QString filepath_file=root_prog_file+"//"+name;
                QDir dir_file(filepath_file);//加载对应路径
                dir_file.setFilter(QDir::Files);//过滤配置文件，接受文件
                QFileInfoList list_file=dir_file.entryInfoList();
                QStringList fileList_file;
                for(int i=0;i<list_file.count();i++){
                    QString filename_name=list_file.at(i).fileName();
                    int len=filename_name.length();
                    QString insert_name=filename_name.left(len-5);
                    QStandardItem* item_file=new QStandardItem(insert_name);//插入文件节点
                    item_project->appendRow(item_file);
                }
            }
            //当root中有节点的话，判断
            else{
                int count=0;//通过count来判断当前想插入的项目名称，root中是否有相同的
                //遍历root下每个项目的名称，看现在想加入的项目名称是否有和root下重名的
                for(int i=0;i<item_root_project->rowCount();i++){
                    QModelIndex index=treemodel_project->index(i,0,QModelIndex());
                    //判断添加到model下的项目名称是否有和当前相加的项目名称重名的
                    if(name==treemodel_project->data(index,Qt::EditRole).toString()){
                        count++;//若重名的话，count+1
                        break;//跳出循环，继续下一次的循环
                    }
                }
                //如果count还为0，证明当前想加入的项目名称和root下的没有重名的
                if(count==0){
                    QStandardItem* item_project=new QStandardItem(name);//插入项目节点
                    item_root_project->appendRow(item_project);//将项目名称添加进节点中
                    QString filepath_file=root_prog_file+"//"+name;
                    QDir dir_file(filepath_file);//加载对应路径
                    dir_file.setFilter(QDir::Files);//过滤配置文件，接受文件
                    QFileInfoList list_file=dir_file.entryInfoList();
                    QStringList fileList_file;
                    for(int i=0;i<list_file.count();i++){
                        QString filename_name=list_file.at(i).fileName();
                        int len=filename_name.length();
                        QString insert_name=filename_name.left(len-5);
                        QStandardItem* item_file=new QStandardItem(insert_name);//插入文件节点
                        item_project->appendRow(item_file);
                    }
                }

            }
        }
    }
    //最后，展开所有节点
    ui->treeView_project->expandAll();

}
void Widget::on_btn_program_charge_clicked()
{
    int indexpage=4;
    switch_page(indexpage);
    int indexpage2=0;
    switch_page_program(indexpage2);
    cout<<"program_charge_edit_widget"<<endl;
}
void Widget::on_btn_coordinate_charge_clicked()
{
    int indexpage=5;
    switch_page(indexpage);
    cout<<"coordinate_show_widget"<<endl;
}
void Widget::on_btn_information_report_charge_clicked()
{
    int indexpage=6;
    switch_page(indexpage);
    cout<<"information_report_charge"<<endl;
}
//真正切换模块界面的槽函数
void Widget::switch_page(int a){
    ui->stackedWidget->setCurrentIndex(a);
}
//程序管理模块：新建按钮切换至指令选择界面
void Widget::on_btn_program_new_clicked()
{
    int indexpage=1;
    switch_page_program(indexpage);
    cout<<"program_charge_choose_widget"<<endl;
    //进入指令选择界面的布局
    set_instruction_choose();
}
//程序管理模块：指令选择界面，确定按钮切换到指令设置界面
void Widget::on_btn_instruction_confirm_clicked()
{
    //当选择运动指令组的时候，切换到指令设置界面
    QModelIndex index=ui->treeView_instruction_choose->currentIndex();
    QStandardItem* item=treemodel_instruction_choose->itemFromIndex(index);
    QStandardItem* item_parent=item->parent();
    if(item_parent->text()=="运动指令组"|item_parent->text()=="回零指令组"|item_parent->text()=="设置指令组"|item_parent->text()=="系统功能指令组"){
        if(item->text()!="StopRobot"&&item->text()!="Stop"&&item->text()!="...=...(赋值)"&&item->text()!="//...(注释)"){
            int indexpage=2;
            switch_page_program(indexpage);
            cout<<"program_charge_set_widget"<<endl;
        }
    }
    set_current_instruction();//选中对应的指令之后，进入对应指令的指令设置界面
    //当选中treeview_instruction_set时，控制程序管理模块按钮是否可以点击
    connect(ui->treeView_instruction_set->selectionModel(),&QItemSelectionModel::currentChanged,this,&Widget::slot_treeview_instruction_set_CurrentChanged);

}
//程序管理模块：指令选择的返回按钮，切换为程序编辑界面
void Widget::on_btn_instruction_back_clicked()
{
    int indexpage=0;
    switch_page_program(indexpage);
    cout<<"program_charge_edit_widget"<<endl;
}
//程序管理模块：指令设置界面的确定按钮，返回程序编辑界面
void Widget::on_btn_instruction_confirm_2_clicked()
{
    int indexpage=0;
    switch_page_program(indexpage);
    cout<<"program_charge_edit_widget"<<endl;
    set_instruction_to_program_edit();//程序管理模块中指令设置界面的确定按钮：将指令插入文本中
}
//程序管理模块：指令设置的返回按钮，切换为指令选择界面
void Widget::on_btn_instruction_back_2_clicked()
{
    int indexpage=1;
    switch_page_program(indexpage);
    cout<<"program_charge_choose_widget"<<endl;
}
//程序管理模块中：stackedwidget真正切换界面的槽函数
void Widget::switch_page_program(int a){
    ui->stackedWidget_page_program->setCurrentIndex(a);
}
//伺服上电按钮的槽函数
void Widget::on_btn_pwr_clicked()
{
    //点击pwr,伺服下电
    if(ui->btn_pwr->isChecked()){
        ui->label_pwr->setText("PWR_ON");
        ui->label_pwr->setStyleSheet("QLabel{background-color:rgba(0,105,0,100%);\color: white;}");
        enableState = 1;
    }
    //点击pwr，伺服上电
    else{
        ui->label_pwr->setText("PWR_OFF");
        ui->label_pwr->setStyleSheet("QLabel{background-color:rgba(105,105,105,100%);\color: white;}");
        enableState = 0;
    }
}
//开始按钮的槽函数
void Widget::on_btn_start_clicked()
{
    if (runState == 0)
    {
        if (hasLoad != 1)
        {
            QMessageBox::warning(this,"开始运行程序","尚未加载程序，请先加载程序后再开始运行");
            return;
        }
        else
        {
            //满足程序运行条件后，将机器人运行状态标志置为1
            if (operateMode == 1)
            {
                //当操作模式为自动时，点击“开始”键，runState才置为1
                runState = 1;
            }
        }
    }
}

void Widget::on_btn_start_pressed()
{
    if (runState == 0)
    {
        if (hasLoad != 1)
        {
            QMessageBox::warning(this,"开始运行程序","尚未加载程序，请先加载程序后再开始运行");
            return;
        }
        else
        {
            //满足程序运行条件后，将机器人运行状态标志置为1
            if (operateMode == 0)
            {
                //当操作模式为手动时，要一直按着“开始”键，runState才置为1
                runState = 1;
            }
        }
    }
}

void Widget::on_btn_start_released()
{
    if (runState == 1)
    {
        if (operateMode == 0)
        {
            //当操作模式为手动时，松开“开始”键后，runState置为0
            runState = 0;
        }
    }
}

//停止按钮的槽函数
void Widget::on_btn_stop_clicked()
{
    if (runState == 1)
    {
        if (hasLoad != 1)
        {
            QMessageBox::warning(this,"停止运行程序","尚未加载程序，当前没有正在运行的程序");
        }
        else
        {
            runState = 0;
        }
    }
}
//切换坐标系按钮的槽函数
void Widget::on_btn_jog_clicked()
{
    //0：直角坐标系，1：关节坐标系，2：工具手坐标系
    if(count_coordinate == 0)
    {
        count_coordinate = 1;
        //直角坐标系
        ui->label_coordinate1->setText("X");
        ui->label_coordinate2->setText("Y");
        ui->label_coordinate3->setText("Z");
        ui->label_coordinate4->setText("A");
        ui->label_coordinate5->setText("B");
        ui->label_coordinate6->setText("C");
        jog = 1;
    }
    else if(count_coordinate == 1)
    {
        count_coordinate = 2;
        //工具手坐标系
        ui->label_coordinate1->setText("TX");
        ui->label_coordinate2->setText("TY");
        ui->label_coordinate3->setText("TZ");
        ui->label_coordinate4->setText("TA");
        ui->label_coordinate5->setText("TB");
        ui->label_coordinate6->setText("TC");
        jog = 2;
    }
    else
    {
        count_coordinate = 0;
        //关节坐标系
        ui->label_coordinate1->setText("A1");
        ui->label_coordinate2->setText("A2");
        ui->label_coordinate3->setText("A3");
        ui->label_coordinate4->setText("A4");
        ui->label_coordinate5->setText("A5");
        ui->label_coordinate6->setText("A6");
        jog = 0;
    }
}
//连续/单步按钮的槽函数
void Widget::on_btn_step_clicked()
{
    //点击step按钮，切换到程序执行模式
    if(ui->btn_step->isChecked()){
        ui->label_step->setText("单步");
        step = 1;
    }
    else{
        ui->label_step->setText("连续");
        step = 0;
    }
}
//手动/自动按钮的槽函数
void Widget::on_btn_hand_auto_clicked()
{
    //0：自动模式，1：手动模式，2为远程模式
    if(count_auto == 0)
    {
        count_auto = 1;
        //自动模式
        ui->label_mode->setText("模式:A");
        operateMode = 1;
    }
    else if(count_auto == 1)
    {
        count_auto = 0;
        //手动模式
        ui->label_mode->setText("模式:T");
        operateMode = 0;
    }
}
//V-按钮的槽函数
void Widget::on_btn_V_down_clicked()
{

    QString temp1=ui->label_robot_speed->text();//获取当前label显示的速度值
    int len=temp1.length();//获取字符串的长度
    QString temp2=temp1.left(len-1);//获取数字的字符串
    int temp3=temp2.toInt();//转为Int类型
    if(temp3>=0){
        temp3--;
        QString temp4=QString::number(temp3);
        QString temp5=temp4+"%";
        ui->label_robot_speed->setText(temp5);
        //设置相对最大运动参数的百分比
        ovr = temp3;
    }

}
//V+按钮的槽函数
void Widget::on_btn_V_up_clicked()
{
    QString temp1=ui->label_robot_speed->text();//获取当前label显示的速度值
    int len=temp1.length();//获取字符串的长度
    QString temp2=temp1.left(len-1);//获取数字的字符串
    int temp3=temp2.toInt();//转为Int类型
    if(temp3<=100){
        temp3++;
        QString temp4=QString::number(temp3);
        QString temp5=temp4+"%";
        ui->label_robot_speed->setText(temp5);
        //设置相对最大运动参数的百分比
        ovr = temp3;
    }
}

//手动示教时，-+按钮的槽函数
void Widget::on_btn_coordinate1_down_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 0;
        coordinate = 1;
    }
}

void Widget::on_btn_coordinate1_down_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate1_up_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 1;
        coordinate = 1;
    }
}

void Widget::on_btn_coordinate1_up_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate2_down_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 0;
        coordinate = 2;
    }
}

void Widget::on_btn_coordinate2_down_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate2_up_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 1;
        coordinate = 2;
    }
}

void Widget::on_btn_coordinate2_up_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate3_down_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 0;
        coordinate = 3;
    }
}

void Widget::on_btn_coordinate3_down_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate3_up_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 1;
        coordinate = 3;
    }
}

void Widget::on_btn_coordinate3_up_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate4_down_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 0;
        coordinate = 4;
    }
}

void Widget::on_btn_coordinate4_down_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate4_up_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 1;
        coordinate = 4;
    }
}

void Widget::on_btn_coordinate4_up_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate5_down_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 0;
        coordinate = 5;
    }
}

void Widget::on_btn_coordinate5_down_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate5_up_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 1;
        coordinate = 5;
    }
}

void Widget::on_btn_coordinate5_up_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate6_down_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 0;
        coordinate = 6;
    }
}

void Widget::on_btn_coordinate6_down_released()
{
    coordinate = 0;
}

void Widget::on_btn_coordinate6_up_pressed()
{
    if(hasReachSingularity != 1)
    {
        upOrDown = 1;
        coordinate = 6;
    }
}

void Widget::on_btn_coordinate6_up_released()
{
    coordinate = 0;
}

//程序管理模块:第一个编辑按钮的槽函数
void Widget::on_btn_program_edit_clicked(bool checked)
{
    if(ui->btn_program_edit->isChecked()){
        //一开始为不可编辑状态，此时为可编辑状态
        ui->btn_program_edit->setText("确认");
        ui->program_textEdit->setReadOnly(false);//进入编辑状态
    }
    else{
        //进入不可编辑的状态
        ui->btn_program_edit->setText("编辑");
        ui->program_textEdit->setReadOnly(true);//进入只读状态
        //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
        //若当前选中文本，需要先将文本移动到文末
        int temp=line_program_edit;//保存文本当前的行数
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::End);
        cur.select(QTextCursor::BlockUnderCursor);
        cur.removeSelectedText();
        //删除完行数之后，将光标移动到刚指定的行上面
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
        ui->program_textEdit->setTextCursor(QTextCursor(block));

        //将文本数据框取出并按行排列
        QModelIndex current_index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
        QStandardItem* parent_item=current_item->parent();//获取父亲item
        QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
        QFile file(file_path);
        if(!file.open(QFile::WriteOnly|QFile::Text)){
            QMessageBox::information(this,"错误信息","当前程序无法保存");
        }
        else{
            QTextStream out(&file);//分行写入文件
            out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
            out<<"{"<<"\n";
            out<<ui->program_textEdit->toPlainText()<<"\n";
            out<<"}";
        }
        //将文本写入文件之后，再在文本最后插入<<END_OFF>>
        ui->program_textEdit->append("<<END_OFF>>");
    }
}
//程序管理模块：指令修改按钮的槽函数
void Widget::on_btn_program_hong_clicked()
{
    //获取光标所在行得内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    QString instruction=cur.selectedText();
    QString judge=instruction.left(instruction.indexOf("(",1));
    //根据不同得指令，进入到不同得指令设置界面
    if(judge=="PTP"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置PTP指令
        treemodel_PTP=new QStandardItemModel(0,2,ui->page_program);
        treemodel_PTP->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_PTP->setHeaderData(1,Qt::Horizontal,"数值");
        root_PTP=treemodel_PTP->invisibleRootItem();//设置根节点
        QStandardItem* item_PTP=new QStandardItem("PTP");//将PTP插入其中
        root_PTP->appendRow(item_PTP);

        //插入pos参数信息
        QStandardItem* item_PTP_pos=new QStandardItem("pos:POSITION_");
        QStandardItem* item_PTP_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_PTP->appendRow(item_PTP_pos);
        item_PTP->appendRow(item_PTP_dyn);
        ui->treeView_program_correct->setModel(treemodel_PTP);//在树中显示Model
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //需要解析一次xml文件，看xml文件是否有子节点，即是否有变量，若有的话，需要加到相应的参数节点之下供用户选择
        set_xml_to_PTP_Lin_Circ(root_PTP);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_PTP);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_PTP);

    }
    if(judge=="Lin"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置Lin指令
        treemodel_Lin=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Lin->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Lin->setHeaderData(1,Qt::Horizontal,"数值");
        root_Lin=treemodel_Lin->invisibleRootItem();//设置根节点
        QStandardItem* item_Lin=new QStandardItem("Lin");//将Lin插入其中
        root_Lin->appendRow(item_Lin);

        //插入pos参数
        QStandardItem* item_Lin_pos=new QStandardItem("pos:POSITION_");
        QStandardItem* item_Lin_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_Lin->appendRow(item_Lin_pos);
        item_Lin->appendRow(item_Lin_dyn);
        ui->treeView_program_correct->setModel(treemodel_Lin);//在树中显示Model
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);

        //解析Lin_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ(root_Lin);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_Lin);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_Lin);
    }
    if(judge=="Circ"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置Circ指令
        treemodel_Circ=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Circ->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Circ->setHeaderData(1,Qt::Horizontal,"数值");
        root_Circ=treemodel_Circ->invisibleRootItem();//设置根节点
        QStandardItem* item_Circ=new QStandardItem("Circ");//将Circ插入其中
        root_Circ->appendRow(item_Circ);

        //插入pos参数
        QStandardItem* item_Circ_pos=new QStandardItem("circPos:POSITION_");
        QStandardItem* item_Circ_dyn=new QStandardItem("dyn_DYNAMIC_(OPT)");
        item_Circ->appendRow(item_Circ_pos);
        item_Circ->appendRow(item_Circ_dyn);
        ui->treeView_program_correct->setModel(treemodel_Circ);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);

        //解析Circ_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ(root_Circ);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_Circ);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_Circ);

    }
    if(judge=="PTPRel"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置PTPRel指令
        treemodel_PTPrel=new QStandardItemModel(0,2,ui->page_program);
        treemodel_PTPrel->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_PTPrel->setHeaderData(1,Qt::Horizontal,"数值");
        root_PTPrel=treemodel_PTPrel->invisibleRootItem();//设置根节点
        QStandardItem* ITEM_ptpREL=new QStandardItem("PTPRel");
        root_PTPrel->appendRow(ITEM_ptpREL);
        //插入相关的参数
        QStandardItem* item_dist_pos=new QStandardItem("dist:DISTANCE_");
        QStandardItem* item_dist_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        ITEM_ptpREL->appendRow(item_dist_pos);
        ITEM_ptpREL->appendRow(item_dist_dyn);
        ui->treeView_program_correct->setModel(treemodel_PTPrel);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析Circ_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ(root_PTPrel);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_PTPrel);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_PTPrel);
    }
    if(judge=="LinRel"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置LinRel指令
        treemodel_Linrel=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Linrel->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Linrel->setHeaderData(1,Qt::Horizontal,"数值");
        root_LinRel=treemodel_Linrel->invisibleRootItem();
        QStandardItem* item_linrel=new QStandardItem("LinRel");
        root_LinRel->appendRow(item_linrel);
        //插入相关的参数
        QStandardItem* item_dist_pos=new QStandardItem("dist:DISTANCE_");
        QStandardItem* item_dist_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_linrel->appendRow(item_dist_pos);
        item_linrel->appendRow(item_dist_dyn);
        ui->treeView_program_correct->setModel(treemodel_Linrel);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析Circ_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ( root_LinRel);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_LinRel);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_LinRel);
    }
    if(judge=="StopRobot"){
        //开始设置stoprobot指令
        treemodel_stoprobot=new QStandardItemModel(0,2,ui->page_program);
        treemodel_stoprobot->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_stoprobot->setHeaderData(1,Qt::Horizontal,"数值");
        root_stoprobot=treemodel_stoprobot->invisibleRootItem();//设置根节点
        QStandardItem* item_stoprobot=new QStandardItem("StopRobot");
        root_stoprobot->appendRow(item_stoprobot);
        QStandardItem* item_mode=new QStandardItem("StopMode");
        item_stoprobot->appendRow(item_mode);
        //插入参数信息
        QStandardItem* item_H=new QStandardItem("HARDSTOP");
        QStandardItem* item_S=new QStandardItem("SOFTSTOP");
        item_mode->appendRow(item_H);
        item_mode->appendRow(item_S);
        ui->treeView_program_correct->setModel(treemodel_stoprobot);//在树中显示Model
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //跳转到指令设置界面
        int indexpage=3;
        switch_page_program(indexpage);
    }
    if(judge=="RefRobotAxis"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置RefRobotAxis指令
        treemodel_refrobotaxis=new QStandardItemModel(0,2,ui->page_program);
        treemodel_refrobotaxis->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_refrobotaxis->setHeaderData(1,Qt::Horizontal,"数值");
        root_refrobotaxis=treemodel_refrobotaxis->invisibleRootItem();
        QStandardItem* item_refaroboraxis=new QStandardItem("RefRobotAxis");
        root_refrobotaxis->appendRow(item_refaroboraxis);
        //插入相关的参数
        QStandardItem* item_axis=new QStandardItem("axis:ROBOTAXIS");
        QStandardItem* item_addMoveTarget=new QStandardItem("addMoveTarget:REAL(OPT)");
        QStandardItem* item_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_refaroboraxis->appendRow(item_axis);
        item_refaroboraxis->appendRow(item_addMoveTarget);
        item_refaroboraxis->appendRow(item_dyn);
        QStandardItem* item_a1=new QStandardItem("A1");
        QStandardItem* item_a2=new QStandardItem("A2");
        QStandardItem* item_a3=new QStandardItem("A3");
        QStandardItem* item_a4=new QStandardItem("A4");
        QStandardItem* item_a5=new QStandardItem("A5");
        QStandardItem* item_a6=new QStandardItem("A6");
        item_axis->appendRow(item_a1);
        item_axis->appendRow(item_a2);
        item_axis->appendRow(item_a3);
        item_axis->appendRow(item_a4);
        item_axis->appendRow(item_a5);
        item_axis->appendRow(item_a6);
        ui->treeView_program_correct->setModel(treemodel_refrobotaxis);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析变量全局的xml文件，显示到refrobot的模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_refrobotaxis,all_path);
        //解析项目的xml文件，显示到refrobot模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_refrobotaxis,project_path);
        //解析程序的xml文件，显示到refrobot模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_different_range_xml_to_refrobot_model(root_refrobotaxis,program_path);
    }
    if(judge=="RefRobotAxisAsync"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置RefRobotAxisAsync指令
        treemodel_RefRobotAxisAsync=new QStandardItemModel(0,2,ui->page_program);
        treemodel_RefRobotAxisAsync->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_RefRobotAxisAsync->setHeaderData(1,Qt::Horizontal,"数值");
        root_RefRobotAxisAsync=treemodel_RefRobotAxisAsync->invisibleRootItem();
        QStandardItem* item_RefRobotAxisAsync=new QStandardItem("RefRobotAxisAsync");
        root_RefRobotAxisAsync->appendRow(item_RefRobotAxisAsync);
        //插入相关的参数
        QStandardItem* item_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_RefRobotAxisAsync->appendRow(item_dyn);
        ui->treeView_program_correct->setModel(treemodel_RefRobotAxisAsync);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析变量全局的xml文件，显示到refrobot的模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_different_range_xml_to_refrobotAxis_model(root_RefRobotAxisAsync,all_path);
        //解析项目的xml文件，显示到refrobot模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_different_range_xml_to_refrobotAxis_model(root_RefRobotAxisAsync,project_path);
        //解析程序的xml文件，显示到refrobot模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_different_range_xml_to_refrobotAxis_model(root_RefRobotAxisAsync,program_path);
    }
    if(judge=="MoveRobotAxis"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置MoveRobotAxis指令
        treemdoel_MoveRobotAxis=new QStandardItemModel(0,2,ui->page_program);
        treemdoel_MoveRobotAxis->setHeaderData(0,Qt::Horizontal,"名字");
        treemdoel_MoveRobotAxis->setHeaderData(1,Qt::Horizontal,"数值");
        root_MoveRobotAxis=treemdoel_MoveRobotAxis->invisibleRootItem();
        QStandardItem* item_move=new QStandardItem("MoveRobotAxis");
        root_MoveRobotAxis->appendRow(item_move);
        //插入相关的参数
        QStandardItem* item_axis=new QStandardItem("axis:ROBOTAXIS");
        QStandardItem* item_pos=new QStandardItem("pos:REAL");
        QStandardItem* item_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_move->appendRow(item_axis);
        item_move->appendRow(item_pos);
        item_move->appendRow(item_dyn);
        QStandardItem* item_a1=new QStandardItem("A1");
        QStandardItem* item_a2=new QStandardItem("A2");
        QStandardItem* item_a3=new QStandardItem("A3");
        QStandardItem* item_a4=new QStandardItem("A4");
        QStandardItem* item_a5=new QStandardItem("A5");
        QStandardItem* item_a6=new QStandardItem("A6");
        item_axis->appendRow(item_a1);
        item_axis->appendRow(item_a2);
        item_axis->appendRow(item_a3);
        item_axis->appendRow(item_a4);
        item_axis->appendRow(item_a5);
        item_axis->appendRow(item_a6);
        ui->treeView_program_correct->setModel(treemdoel_MoveRobotAxis);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析变量全局的xml文件，显示到refrobot的模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_MoveRobotAxis,all_path);
        //解析项目的xml文件，显示到refrobot模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_MoveRobotAxis,project_path);
        //解析程序的xml文件，显示到refrobot模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_different_range_xml_to_refrobot_model(root_MoveRobotAxis,program_path);
    }
    if(judge=="Dyn"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置Dyn指令
        treemodel_dyn=new QStandardItemModel(0,2,ui->page_program);
        treemodel_dyn->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_dyn->setHeaderData(1,Qt::Horizontal,"数值");
        root_dyn=treemodel_dyn->invisibleRootItem();
        QStandardItem* item_dyn=new QStandardItem("Dyn");
        root_dyn->appendRow(item_dyn);
        //插入相关的参数
        QStandardItem* item_d=new QStandardItem("dyn:DYNAMIC");
        item_dyn->appendRow(item_d);
        ui->treeView_program_correct->setModel(treemodel_dyn);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析DYNAMIC参数到节点中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_dynamic_variable_to_dyn_model(root_dyn,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_dynamic_variable_to_dyn_model(root_dyn,project_path);
        //解析程序的xml文件，显示到dyn模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_dynamic_variable_to_dyn_model(root_dyn,program_path);

    }
    if(judge=="DynOvr"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置Dynovr指令
        treemdoel_dynovr=new QStandardItemModel(0,2,ui->page_program);
        treemdoel_dynovr->setHeaderData(0,Qt::Horizontal,"名字");
        treemdoel_dynovr->setHeaderData(1,Qt::Horizontal,"数值");
        root_dynovr=treemdoel_dynovr->invisibleRootItem();
        QStandardItem* item_dynovr=new QStandardItem("DynOvr");
        root_dynovr->appendRow(item_dynovr);
        ui->treeView_program_correct->setModel(treemdoel_dynovr);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析整数和浮点型变量到节点中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_IF_variable_to_dynovr_model(root_dynovr,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_IF_variable_to_dynovr_model(root_dynovr,project_path);
        //解析程序的xml文件，显示到dyn模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_IF_variable_to_dynovr_model(root_dynovr,program_path);
    }
    if(judge=="Ramp"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置ramp指令
        treemodel_Ramp=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Ramp->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Ramp->setHeaderData(1,Qt::Horizontal,"数值");
        root_Ramp=treemodel_Ramp->invisibleRootItem();
        QStandardItem* item_ramp=new QStandardItem("Ramp");
        root_Ramp->appendRow(item_ramp);
        QStandardItem* item_ramp_type=new QStandardItem("type:RAMPTYPE");
        item_ramp->appendRow(item_ramp_type);
        //往type中添加类型item
        QStandardItem* item_T=new QStandardItem("T");
        QStandardItem* item_S=new QStandardItem("S");
        item_ramp_type->appendRow(item_T);
        item_ramp_type->appendRow(item_S);
        ui->treeView_program_correct->setModel(treemodel_Ramp);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
    }
    if(judge=="RefSys"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置Refsys指令
        treemodel_Refsys=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Refsys->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Refsys->setHeaderData(1,Qt::Horizontal,"数值");
        root_Refsys=treemodel_Refsys->invisibleRootItem();
        QStandardItem* item_Refsys=new QStandardItem("RefSys");
        root_Refsys->appendRow(item_Refsys);
        QStandardItem* item_sys=new QStandardItem("refSys:REFSYS_");
        item_Refsys->appendRow(item_sys);
        ui->treeView_program_correct->setModel(treemodel_Refsys);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析CARTREFSYS数据类型到节点中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_Variable_to_Refsys_model(root_Refsys,all_path);//解析全局xml文件
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_Variable_to_Refsys_model(root_Refsys,project_path);
        //解析程序的xml文件，显示到dyn模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_Variable_to_Refsys_model(root_Refsys,program_path);
    }
    if(judge=="Tool"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置Tool指令
        treemodel_tool=new QStandardItemModel(0,2,ui->page_program);
        treemodel_tool->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_tool->setHeaderData(1,Qt::Horizontal,"数值");
        root_tool=treemodel_tool->invisibleRootItem();
        QStandardItem* item_tool=new QStandardItem("Tool");
        root_tool->appendRow(item_tool);
        QStandardItem* item_tool_t=new QStandardItem("tool:TOOL_");
        item_tool->appendRow(item_tool_t);
        ui->treeView_program_correct->setModel(treemodel_tool);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析关节坐标系和世界坐标系的数据类型
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_variable_to_tool_model(root_tool,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_variable_to_tool_model(root_tool,project_path);
        //解析程序的xml文件，显示到tool模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_variable_to_tool_model(root_tool,program_path);
    }
    //系统功能指令组
    //赋值指令需要不同的字符判断标志
    if(instruction.contains("=")&&(!instruction.contains("WHILE"))&&(!instruction.contains("LABEL:"))){
        //指令包含=号，证明为赋值指令
        //新建对话框
        QDialog* fuzhi_dialog=new QDialog(ui->page_program);
        fuzhi_dialog->setWindowTitle("用户设置");
        QFormLayout form(fuzhi_dialog);
        //创建一个label和edit
        QString tf1=QString("变量");
        QLineEdit* edit1=new QLineEdit(ui->page_program);
        form.addRow(tf1,edit1);
        //再创建一个label和edit
        QString tf2=QString("赋值");
        QLineEdit* edit2=new QLineEdit(ui->page_program);
        form.addRow(tf2,edit2);
        //添加OK和CANCLE按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,fuzhi_dialog);
        form.addRow(&buttonBox);
        //当点击OK时
        //当点击OK时
        connect(&buttonBox,SIGNAL(accepted()),fuzhi_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),fuzhi_dialog,SLOT(reject()));
        //当点击OK时
        if(fuzhi_dialog->exec()==QDialog::Accepted){
            //获取用户输入的语句
            QString tf_name=edit1->text();
            QString tf_value=edit2->text();
            QString name=tf_name+"="+tf_value;
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }
    }
    if(judge=="WaitTime"){
        //跳转到waittime设置指令
        int indexpage=3;
        switch_page_program(indexpage);
        treemodel_waittime=new QStandardItemModel(0,2,ui->page_program);
        treemodel_waittime->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_waittime->setHeaderData(1,Qt::Horizontal,"数值");
        root_waittime=treemodel_waittime->invisibleRootItem();
        QStandardItem* item_waittime=new QStandardItem("WaitTime");
        root_waittime->appendRow(item_waittime);
        QStandardItem* item_dint=new QStandardItem("timeMs:DINT");
        item_waittime->appendRow(item_dint);
        ui->treeView_program_correct->setModel(treemodel_waittime);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析浮点数和整数的数据类型
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_variable_to_waittime_model(root_waittime,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_variable_to_waittime_model(root_waittime,project_path);
        //解析程序的xml文件，显示到tool模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_variable_to_waittime_model(root_waittime,program_path);

    }
    if(judge=="Stop"){
        //开始设置stoprobot指令
        treemodel_stop=new QStandardItemModel(0,2,ui->page_program);
        treemodel_stop->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_stop->setHeaderData(1,Qt::Horizontal,"数值");
        root_stop=treemodel_stop->invisibleRootItem();//设置根节点
        QStandardItem* item_stoprobot=new QStandardItem("Stop");
        root_stop->appendRow(item_stoprobot);
        QStandardItem* item_mode=new QStandardItem("StopMode");
        item_stoprobot->appendRow(item_mode);
        //插入参数信息
        QStandardItem* item_H=new QStandardItem("HARDSTOP");
        QStandardItem* item_S=new QStandardItem("SOFTSTOP");
        item_mode->appendRow(item_H);
        item_mode->appendRow(item_S);
        ui->treeView_program_correct->setModel(treemodel_stop);//在树中显示Model
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //跳转到指令设置界面
        int indexpage=3;
        switch_page_program(indexpage);
    }
    if(judge=="Notice"){
        //跳转到Notice指令
        int indexpage=3;
        switch_page_program(indexpage);
        treemodel_info=new QStandardItemModel(0,2,ui->page_program);
        treemodel_info->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_info->setHeaderData(1,Qt::Horizontal,"数值");
        root_info=treemodel_info->invisibleRootItem();
        QStandardItem* item_info=new QStandardItem("Notice");
        root_info->appendRow(item_info);
        QStandardItem* item_str=new QStandardItem("text:STRING");
        QStandardItem* item_p1=new QStandardItem("param1:ANY(OPT)");
        QStandardItem* item_p2=new QStandardItem("param2:ANY(OPT)");
        item_info->appendRow(item_str);
        item_info->appendRow(item_p1);
        item_info->appendRow(item_p2);
        ui->treeView_program_correct->setModel(treemodel_info);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析字符串类型的数据到模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_string_variable_to_info_model(root_info,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_string_variable_to_info_model(root_info,project_path);
        //解析程序的xml文件，显示到模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_string_variable_to_info_model(root_info,program_path);
    }
    if(judge=="Warning"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置警告指令
        treemodel_warning=new QStandardItemModel(0,2,ui->page_program);
        treemodel_warning->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_warning->setHeaderData(1,Qt::Horizontal,"数值");
        root_warning=treemodel_warning->invisibleRootItem();
        QStandardItem* item_warning=new QStandardItem("Warning");
        root_warning->appendRow(item_warning);
        QStandardItem* item_str=new QStandardItem("text:STRING");
        QStandardItem* item_param1=new QStandardItem("param1:ANY(OPT)");
        QStandardItem* item_param2=new QStandardItem("param2:ANY(OPT)");
        item_warning->appendRow(item_str);
        item_warning->appendRow(item_param1);
        item_warning->appendRow(item_param2);
        ui->treeView_program_correct->setModel(treemodel_warning);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析字符串类型得数据到模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_string_variable_to_info_model(root_warning,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_string_variable_to_info_model(root_warning,project_path);
        //解析程序的xml文件，显示到模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_string_variable_to_info_model(root_warning,program_path);
    }
    if(judge=="Error"){
        int indexpage=3;
        switch_page_program(indexpage);
        //开始设置警告指令
        treemodel_error=new QStandardItemModel(0,2,ui->page_program);
        treemodel_error->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_error->setHeaderData(1,Qt::Horizontal,"数值");
        root_error=treemodel_error->invisibleRootItem();
        QStandardItem* item_error=new QStandardItem("Error");
        root_error->appendRow(item_error);
        QStandardItem* item_str=new QStandardItem("text:STRING");
        QStandardItem* item_param1=new QStandardItem("param1:ANY(OPT)");
        QStandardItem* item_param2=new QStandardItem("param2:ANY(OPT)");
        item_error->appendRow(item_str);
        item_error->appendRow(item_param1);
        item_error->appendRow(item_param2);
        ui->treeView_program_correct->setModel(treemodel_error);
        ui->treeView_program_correct->setColumnWidth(0,300);
        ui->treeView_program_correct->setColumnWidth(1,200);
        //展开节点
        ui->treeView_program_correct->expandToDepth(1);
        //解析字符串类型得数据到模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_string_variable_to_info_model(root_error,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_string_variable_to_info_model(root_error,project_path);
        //解析程序的xml文件，显示到模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_string_variable_to_info_model(root_error,program_path);
    }
    //流程控制指令组
    if(instruction.contains("CALL")){
        //新建对话框
        QDialog* CALL_dialog=new QDialog(ui->page_program);
        CALL_dialog->setWindowTitle("用户设置:");
        QFormLayout form(CALL_dialog);

        //输入变量的类型
        QString Coordiante=QString("调用程序:");
        QComboBox* combox=new QComboBox(CALL_dialog);//创建一个下拉框
        //开始遍历对应的文件路径，并且将程序名称写到combox控件上
        //首先获取当前文件路径下的目录路径
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* item=treemodel_project->itemFromIndex(index);
        QStandardItem* item_parent=item->parent();
        QString project_name=item_parent->text();
        QString path=root_prog_file+"//"+project_name;//项目路径
        QDir dir_file(path);
        dir_file.setFilter(QDir::Files);//过滤配置文件
        QFileInfoList list_file=dir_file.entryInfoList();
        QStringList filelist_file;
        for(int i=0;i<list_file.count();i++){
            QString filename_name=list_file.at(i).fileName();
            int len=filename_name.length();
            QString temp=filename_name.left(len-5);
            QString name=temp+"()";
            combox->addItem(name);//将文件名插入combox控件中
        }
        form.addRow(Coordiante,combox);

        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,CALL_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),CALL_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),CALL_dialog,SLOT(reject()));

        //当点击OK按钮时
        if(CALL_dialog->exec()==QDialog::Accepted){
            // 获取combox当前内容
            QString pro_name=combox->currentText();
            QString name="CALL "+pro_name;
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }
    }
    if(instruction.contains("WAIT")){
        //新建一个对话框
        QDialog* WAIT_dialog=new QDialog(ui->page_program);
        WAIT_dialog->setWindowTitle("用户设置:");
        QFormLayout form(WAIT_dialog);
        //创建一个label和combox
        QString tf=QString("条件判断:");
        QLineEdit* edit=new QLineEdit(ui->page_program);
        form.addRow(tf,edit);

        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,WAIT_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),WAIT_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),WAIT_dialog,SLOT(reject()));

        //当点击OK按钮时
        if(WAIT_dialog->exec()==QDialog::Accepted){
            //获取用户输入的语句
            QString tf_name=edit->text();
            QString name="WAIT "+tf_name;
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }
    }
    if((instruction.contains("IF")&&instruction.contains("THEN"))|(instruction.contains("ELSEIF")&&instruction.contains("THEN"))){
        //创建一个对话框
        QDialog* IF_dialog=new QDialog(ui->page_program);
        IF_dialog->setWindowTitle("用户设置");
        QFormLayout form(IF_dialog);
        //创建一个label和combox
        QString tf=QString("条件判断:");
        QLineEdit* edit=new QLineEdit(ui->page_program);
        form.addRow(tf,edit);
        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,IF_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),IF_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),IF_dialog,SLOT(reject()));

        //当点击OK按钮时
        if(IF_dialog->exec()==QDialog::Accepted){
            //获取用户输入的语句
            QString tf_name=edit->text();
            QString name;
            if(instruction.contains("IF")){
                name="IF "+tf_name+" THEN";
            }
            if(instruction.contains("ELSEIF")){
                name="ELSEIF "+tf_name+" THEN";
            }
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }

    }
    if(instruction.contains("WHILE")&&instruction.contains("DO")){
        //新建一个对话框
        QDialog* while_dialog=new QDialog(ui->page_program);
        while_dialog->setWindowTitle("用户设置:");
        QFormLayout form(while_dialog);
        //创建一个label和lineedit
        QString tf=QString("条件判断:");
        QLineEdit* edit=new QLineEdit(ui->page_program);
        form.addRow(tf,edit);

        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,while_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),while_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),while_dialog,SLOT(reject()));

        //当点击OK按钮时
        if(while_dialog->exec()==QDialog::Accepted){
            QString tf_name=edit->text();
            QString name="WHILE "+tf_name+" DO";
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }
    }
    if(instruction.contains("LOOP")&&instruction.contains("DO")){
        //创建一个对话框
        QDialog* LOOP_dialog=new QDialog(ui->page_program);
        LOOP_dialog->setWindowTitle("用户设置:");
        QFormLayout form(LOOP_dialog);
        //创建一个label和edit
        QString loop_value=QString("循环次数:");
        QLineEdit* loop_edit=new QLineEdit(LOOP_dialog);
        form.addRow(loop_value,loop_edit);//布局一下

        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,LOOP_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),LOOP_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),LOOP_dialog,SLOT(reject()));

        //当点击OK按钮时
        if(LOOP_dialog->exec()==QDialog::Accepted){
            QString loop_name=loop_edit->text();
            QString name="LOOP "+loop_name+" DO";
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }
    }
    if(instruction.contains("RUN")|instruction.contains("KILL")){
        //新建对话框
        QDialog* run_dialog=new QDialog(ui->page_program);
        run_dialog->setWindowTitle("用户设置:");
        QFormLayout form(run_dialog);
        //输入变量的类型
        QString Coordinate=QString("选择程序:");
        QComboBox* combox=new QComboBox(run_dialog);//创建一个下拉框
        //开始遍历对应的文件路径，并且将程序名称写到combox控件上
        //首先获取当前文件路径下的目录路径
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* item=treemodel_project->itemFromIndex(index);
        QStandardItem* item_parent=item->parent();
        QString project_name=item_parent->text();
        QString path=root_prog_file+"//"+project_name;//项目路径
        QDir dir_file(path);
        dir_file.setFilter(QDir::Files);//过滤配置文件
        QFileInfoList list_file=dir_file.entryInfoList();
        QStringList filelist_file;
        for(int i=0;i<list_file.count();i++){
            QString filename_name=list_file.at(i).fileName();
            int len=filename_name.length();
            QString temp=filename_name.left(len-5);
            QString name=temp;
            combox->addItem(name);//将文件名插入combox控件中
        }
        form.addRow(Coordinate,combox);


        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,run_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),run_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),run_dialog,SLOT(reject()));

        //当点击OK按钮时
        if(run_dialog->exec()==QDialog::Accepted){
            // 获取combox当前内容
            QString pro_name=combox->currentText();
            QString name;
            if(instruction.contains("RUN")){
                name="RUN "+pro_name+"()";
            }
            if(instruction.contains("KILL")){
                name="KILL "+pro_name+"()";
            }
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }
    }
    if((instruction.contains("IF")&&instruction.contains("LABEL"))){
        //创建一个对话框
        QDialog* goto_dialog=new QDialog(ui->page_program);
        goto_dialog->setWindowTitle("用户设置:");
        QFormLayout form(goto_dialog);
        //创建一个label和edit
        QString tf=QString("条件判断");
        QLineEdit* edit=new QLineEdit(goto_dialog);
        form.addRow(tf,edit);
        //添加OK和Cancle按钮
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,goto_dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox,SIGNAL(accepted()),goto_dialog,SLOT(accept()));
        connect(&buttonBox,SIGNAL(rejected()),goto_dialog,SLOT(reject()));
        //当点击OK按钮时
        if(goto_dialog->exec()==QDialog::Accepted){
            QString tf_name=edit->text();
            QString name;
            if(instruction.contains("IF")){
                name="IF "+tf_name+" GOTO"+" LABEL";
            }
            //删除光标所在行内容
            cur.removeSelectedText();
            //删除完成后，添加新的指令内容
            QModelIndex index=ui->treeView_program_correct->currentIndex();
            cur.insertText(name);
            //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
            //若当前选中文本，需要先将文本移动到文末
            int temp=line_program_edit;//保存文本当前的行数
            cur.movePosition(QTextCursor::End);
            cur.select(QTextCursor::BlockUnderCursor);
            cur.removeSelectedText();
            //删除完行数之后，将光标移动到刚指定的行上面
            QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
            ui->program_textEdit->setTextCursor(QTextCursor(block));

            //将文本数据框取出并按行排列
            QModelIndex current_index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
            QStandardItem* parent_item=current_item->parent();//获取父亲item
            QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
            QFile file(file_path);
            if(!file.open(QFile::WriteOnly|QFile::Text)){
                QMessageBox::information(this,"错误信息","当前程序无法保存");
            }
            else{
                QTextStream out(&file);//分行写入文件
                out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
                out<<"{"<<"\n";
                out<<ui->program_textEdit->toPlainText()<<"\n";
                out<<"}";
            }
            //将文本写入文件之后，再在文本最后插入<<END_OFF>>
            ui->program_textEdit->append("<<END_OFF>>");
        }
    }
    //当选中treeview_instruction_set时，控制程序管理模块按钮是否可以点击
    connect(ui->treeView_program_correct->selectionModel(),&QItemSelectionModel::currentChanged,this,&Widget::slot_treeview_instruction_program_correct_CurrentChanged);
}
//程序管理模块：布局指令选择界面的函数
void Widget::set_instruction_choose(){
    //布局视图和模型
    treemodel_instruction_choose=new QStandardItemModel(0,2,ui->page_program);
    treemodel_instruction_choose->setHeaderData(0,Qt::Horizontal,"指令类型");
    treemodel_instruction_choose->setHeaderData(1,Qt::Horizontal,"解释");
    item_root_instruction_choose=treemodel_instruction_choose->invisibleRootItem();//根节点
    ui->treeView_instruction_choose->setModel(treemodel_instruction_choose);//在树中显示Model
    ui->treeView_instruction_choose->setColumnWidth(0,300);
    ui->treeView_instruction_choose->setColumnWidth(1,200);
    //开始插入运动指令组和流程指令控制组
    QStandardItem* item_motion=new QStandardItem("运动指令组");
    QStandardItem* item_set_instruction=new QStandardItem("设置指令组");
    QStandardItem* item_system_function=new QStandardItem("系统功能指令组");
    QStandardItem* item_process_control=new QStandardItem("流程控制指令组");
    item_root_instruction_choose->appendRow(item_motion);//将运动指令组item插入其中
    item_root_instruction_choose->appendRow(item_set_instruction);//将设置指令组插入其中
    item_root_instruction_choose->appendRow(item_system_function);//将系统功能指令组插入其中
    item_root_instruction_choose->appendRow(item_process_control);//将流程控制指令组插入其中
    //在运动指令组中插入相应的指令
    QStandardItem* item_motion_PTP=new QStandardItem("PTP");
    QStandardItem* item_motion_Lin=new QStandardItem("Lin");
    QStandardItem* item_motion_Circ=new QStandardItem("Circ");
    QStandardItem* item_motion_PTPRel=new QStandardItem("PTPRel");
    QStandardItem* item_motion_LinRel=new QStandardItem("LinRel");
    QStandardItem* item_motion_StopRobot=new QStandardItem("StopRobot");
    QStandardItem* item_motion_back0=new QStandardItem("回零指令组");
    QStandardItem* item_motion_back0_RefRobotAxis=new QStandardItem("RefRobotAxis");
    QStandardItem* item_motion_back0_RefRobotAxisAsync=new QStandardItem("RefRobotAxisAsync");
    QStandardItem* item_motion_MoveRobotAxis=new QStandardItem("MoveRobotAxis");
    //将以上指令插入运动指令中
    item_motion->appendRow(item_motion_PTP);
    item_motion->appendRow(item_motion_Lin);
    item_motion->appendRow(item_motion_Circ);
    item_motion->appendRow(item_motion_PTPRel);
    item_motion->appendRow(item_motion_LinRel);
    item_motion->appendRow(item_motion_StopRobot);
    item_motion->appendRow(item_motion_back0);
    //回零指令组中再插入相应指令
    item_motion_back0->appendRow(item_motion_back0_RefRobotAxis);
    item_motion_back0->appendRow(item_motion_back0_RefRobotAxisAsync);
    item_motion->appendRow(item_motion_MoveRobotAxis);
    //在流程控制指令组插入相应的指令
    QStandardItem* item_process_control_CALL=new QStandardItem("CALL");
    QStandardItem* item_process_control_WAIT=new QStandardItem("WAIT");
    QStandardItem* item_process_control_IF=new QStandardItem("IF..THEN..END_IF,ELSEIF..THEN,ELSE");
    QStandardItem* item_process_control_while=new QStandardItem("WHILE..DO..END_WHILE");
    QStandardItem* item_process_control_loop=new QStandardItem("LOOP..DO..END_LOOP");
    QStandardItem* item_process_control_RUN_KILL=new QStandardItem("RUN,KILL");
    QStandardItem* item_process_control_return=new QStandardItem("RETURN");
    QStandardItem* item_process_control_GOTO=new QStandardItem("GOTO..,IF..GOTO..,LABEL..");
    //将以上指令插入流程控制
    item_process_control->appendRow(item_process_control_CALL);
    item_process_control->appendRow(item_process_control_WAIT);
    item_process_control->appendRow(item_process_control_IF);
    item_process_control->appendRow(item_process_control_while);
    item_process_control->appendRow(item_process_control_loop);
    item_process_control->appendRow(item_process_control_RUN_KILL);
    item_process_control->appendRow(item_process_control_return);
    item_process_control->appendRow(item_process_control_GOTO);
    //在设置指令组中插入相应的指令
    QStandardItem* item_set_Dyn=new QStandardItem("Dyn");
    QStandardItem* item_set_Dynovr=new QStandardItem("Dynovr");
    QStandardItem* item_set_ramp=new QStandardItem("Ramp");
    QStandardItem* item_set_Refsys=new QStandardItem("Refsys");
    QStandardItem* item_set_tool=new QStandardItem("Tool");
    //将以上指令插入设置指令组中
    item_set_instruction->appendRow(item_set_Dyn);
    item_set_instruction->appendRow(item_set_Dynovr);
    item_set_instruction->appendRow(item_set_ramp);
    item_set_instruction->appendRow(item_set_Refsys);
    item_set_instruction->appendRow(item_set_tool);
    //在系统功能指令组中插入相应的指令
    QStandardItem* item_set_value=new QStandardItem("...=...(赋值)");
    QStandardItem* item_set_zhushi=new QStandardItem("//...(注释)");
    QStandardItem* item_sys_waittime=new QStandardItem("WaitTime");
    QStandardItem* item_sys_stop=new QStandardItem("Stop");
    QStandardItem* item_sys_info=new QStandardItem("Notice");
    QStandardItem* item_sys_warning=new QStandardItem("Warning");
    QStandardItem* item_sys_error=new QStandardItem("Error");
    //将以上指令插入系统功能指令组中
    item_system_function->appendRow(item_set_value);
    item_system_function->appendRow(item_set_zhushi);
    item_system_function->appendRow(item_sys_waittime);
    item_system_function->appendRow(item_sys_stop);
    item_system_function->appendRow(item_sys_info);
    item_system_function->appendRow(item_sys_warning);
    item_system_function->appendRow(item_sys_error);
    //展开所有节点
    ui->treeView_instruction_choose->expandAll();
    //此时还没选择treeview_instruction_choose的节点，确定按钮不可选
    ui->btn_instruction_confirm->setEnabled(false);
    //当选中treeview_instruction_choose时，控制程序管理模块按钮是否可以点击
    connect(ui->treeView_instruction_choose->selectionModel(),&QItemSelectionModel::currentChanged,this,&Widget::slot_treeview_instruction_choose_CurrentChanged);
}

//程序管理模块，选中指令后，进入对应指令的指令设置界面
void Widget::set_current_instruction(){
    QString current_instruction=ui->treeView_instruction_choose->currentIndex().data().toString();//目前选中的指令名称
    if(current_instruction=="PTP"){
        //开始设置PTP指令
        treemodel_PTP=new QStandardItemModel(0,2,ui->page_program);
        treemodel_PTP->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_PTP->setHeaderData(1,Qt::Horizontal,"数值");
        root_PTP=treemodel_PTP->invisibleRootItem();//设置根节点
        QStandardItem* item_PTP=new QStandardItem("PTP");//将PTP插入其中
        root_PTP->appendRow(item_PTP);
        //插入pos参数信息
        QStandardItem* item_PTP_pos=new QStandardItem("pos:POSITION_");
        QStandardItem* item_PTP_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_PTP->appendRow(item_PTP_pos);
        item_PTP->appendRow(item_PTP_dyn);
        ui->treeView_instruction_set->setModel(treemodel_PTP);//在树中显示Model
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);

        //需要解析一次xml文件，看xml文件是否有子节点，即是否有变量，若有的话，需要加到相应的参数节点之下供用户选择
        set_xml_to_PTP_Lin_Circ(root_PTP);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_PTP);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_PTP);
    }
    if(current_instruction=="Lin"){
        //开始设置Lin指令
        treemodel_Lin=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Lin->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Lin->setHeaderData(1,Qt::Horizontal,"数值");
        root_Lin=treemodel_Lin->invisibleRootItem();//设置根节点
        QStandardItem* item_Lin=new QStandardItem("Lin");//将Lin插入其中
        root_Lin->appendRow(item_Lin);

        //插入pos参数
        QStandardItem* item_Lin_pos=new QStandardItem("pos:POSITION_");
        QStandardItem* item_Lin_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_Lin->appendRow(item_Lin_pos);
        item_Lin->appendRow(item_Lin_dyn);
        ui->treeView_instruction_set->setModel(treemodel_Lin);//在树中显示Model
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);

        //解析Lin_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ(root_Lin);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_Lin);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_Lin);
    }
    if(current_instruction=="Circ"){
        //开始设置Circ指令
        treemodel_Circ=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Circ->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Circ->setHeaderData(1,Qt::Horizontal,"数值");
        root_Circ=treemodel_Circ->invisibleRootItem();//设置根节点
        QStandardItem* item_Circ=new QStandardItem("Circ");//将Circ插入其中
        root_Circ->appendRow(item_Circ);

        //插入pos参数
        QStandardItem* item_Circ_pos=new QStandardItem("circPos:POSITION_");
        QStandardItem* item_Circ_dyn=new QStandardItem("dyn_DYNAMIC_(OPT)");
        item_Circ->appendRow(item_Circ_pos);
        item_Circ->appendRow(item_Circ_dyn);
        ui->treeView_instruction_set->setModel(treemodel_Circ);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析Circ_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ(root_Circ);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_Circ);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_Circ);
    }
    if(current_instruction=="PTPRel"){
        //开始设置PTPRel指令
        treemodel_PTPrel=new QStandardItemModel(0,2,ui->page_program);
        treemodel_PTPrel->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_PTPrel->setHeaderData(1,Qt::Horizontal,"数值");
        root_PTPrel=treemodel_PTPrel->invisibleRootItem();//设置根节点
        QStandardItem* ITEM_ptpREL=new QStandardItem("PTPRel");
        root_PTPrel->appendRow(ITEM_ptpREL);
        //插入相关的参数
        QStandardItem* item_dist_pos=new QStandardItem("dist:DISTANCE_");
        QStandardItem* item_dist_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        ITEM_ptpREL->appendRow(item_dist_pos);
        ITEM_ptpREL->appendRow(item_dist_dyn);
        ui->treeView_instruction_set->setModel(treemodel_PTPrel);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析Circ_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ(root_PTPrel);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_PTPrel);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_PTPrel);
    }
    if(current_instruction=="LinRel"){
        //开始设置LinRel指令
        treemodel_Linrel=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Linrel->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Linrel->setHeaderData(1,Qt::Horizontal,"数值");
        root_LinRel=treemodel_Linrel->invisibleRootItem();
        QStandardItem* item_linrel=new QStandardItem("LinRel");
        root_LinRel->appendRow(item_linrel);
        //插入相关的参数
        QStandardItem* item_dist_pos=new QStandardItem("dist:DISTANCE_");
        QStandardItem* item_dist_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_linrel->appendRow(item_dist_pos);
        item_linrel->appendRow(item_dist_dyn);
        ui->treeView_instruction_set->setModel(treemodel_Linrel);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析Circ_xml文件，显示变量
        set_xml_to_PTP_Lin_Circ( root_LinRel);
        //解析项目作用域节点
        set_project_range_xml_to_PTP_Lin_Circ(root_LinRel);
        //解析全局作用域节点
        set_all_range_xml_to_PTP_Lin_Circ(root_LinRel);
    }
    if(current_instruction=="StopRobot"){
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        deal_StopRobot_instruction();//处理stoprobot函数的
    }
    if(current_instruction=="RefRobotAxis"){
        //开始设置RefRobotAxis指令
        treemodel_refrobotaxis=new QStandardItemModel(0,2,ui->page_program);
        treemodel_refrobotaxis->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_refrobotaxis->setHeaderData(1,Qt::Horizontal,"数值");
        root_refrobotaxis=treemodel_refrobotaxis->invisibleRootItem();
        QStandardItem* item_refaroboraxis=new QStandardItem("RefRobotAxis");
        root_refrobotaxis->appendRow(item_refaroboraxis);
        //插入相关的参数
        QStandardItem* item_axis=new QStandardItem("axis:ROBOTAXIS");
        QStandardItem* item_addMoveTarget=new QStandardItem("addMoveTarget:REAL(OPT)");
        QStandardItem* item_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_refaroboraxis->appendRow(item_axis);
        item_refaroboraxis->appendRow(item_addMoveTarget);
        item_refaroboraxis->appendRow(item_dyn);
        QStandardItem* item_a1=new QStandardItem("A1");
        QStandardItem* item_a2=new QStandardItem("A2");
        QStandardItem* item_a3=new QStandardItem("A3");
        QStandardItem* item_a4=new QStandardItem("A4");
        QStandardItem* item_a5=new QStandardItem("A5");
        QStandardItem* item_a6=new QStandardItem("A6");
        item_axis->appendRow(item_a1);
        item_axis->appendRow(item_a2);
        item_axis->appendRow(item_a3);
        item_axis->appendRow(item_a4);
        item_axis->appendRow(item_a5);
        item_axis->appendRow(item_a6);
        ui->treeView_instruction_set->setModel(treemodel_refrobotaxis);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析变量全局的xml文件，显示到refrobot的模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_refrobotaxis,all_path);
        //解析项目的xml文件，显示到refrobot模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_refrobotaxis,project_path);
        //解析程序的xml文件，显示到refrobot模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_different_range_xml_to_refrobot_model(root_refrobotaxis,program_path);
    }
    if(current_instruction=="RefRobotAxisAsync"){
        //开始设置RefRobotAxisAsync指令
        treemodel_RefRobotAxisAsync=new QStandardItemModel(0,2,ui->page_program);
        treemodel_RefRobotAxisAsync->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_RefRobotAxisAsync->setHeaderData(1,Qt::Horizontal,"数值");
        root_RefRobotAxisAsync=treemodel_RefRobotAxisAsync->invisibleRootItem();
        QStandardItem* item_RefRobotAxisAsync=new QStandardItem("RefRobotAxisAsync");
        root_RefRobotAxisAsync->appendRow(item_RefRobotAxisAsync);
        //插入相关的参数
        QStandardItem* item_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_RefRobotAxisAsync->appendRow(item_dyn);
        ui->treeView_instruction_set->setModel(treemodel_RefRobotAxisAsync);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析变量全局的xml文件，显示到refrobot的模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_different_range_xml_to_refrobotAxis_model(root_RefRobotAxisAsync,all_path);
        //解析项目的xml文件，显示到refrobot模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_different_range_xml_to_refrobotAxis_model(root_RefRobotAxisAsync,project_path);
        //解析程序的xml文件，显示到refrobot模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_different_range_xml_to_refrobotAxis_model(root_RefRobotAxisAsync,program_path);
    }
    if(current_instruction=="MoveRobotAxis"){
        //开始设置MoveRobotAxis指令
        treemdoel_MoveRobotAxis=new QStandardItemModel(0,2,ui->page_program);
        treemdoel_MoveRobotAxis->setHeaderData(0,Qt::Horizontal,"名字");
        treemdoel_MoveRobotAxis->setHeaderData(1,Qt::Horizontal,"数值");
        root_MoveRobotAxis=treemdoel_MoveRobotAxis->invisibleRootItem();
        QStandardItem* item_move=new QStandardItem("MoveRobotAxis");
        root_MoveRobotAxis->appendRow(item_move);
        //插入相关的参数
        QStandardItem* item_axis=new QStandardItem("axis:ROBOTAXIS");
        QStandardItem* item_pos=new QStandardItem("pos:REAL");
        QStandardItem* item_dyn=new QStandardItem("dyn:DYNAMIC_(OPT)");
        item_move->appendRow(item_axis);
        item_move->appendRow(item_pos);
        item_move->appendRow(item_dyn);
        QStandardItem* item_a1=new QStandardItem("A1");
        QStandardItem* item_a2=new QStandardItem("A2");
        QStandardItem* item_a3=new QStandardItem("A3");
        QStandardItem* item_a4=new QStandardItem("A4");
        QStandardItem* item_a5=new QStandardItem("A5");
        QStandardItem* item_a6=new QStandardItem("A6");
        item_axis->appendRow(item_a1);
        item_axis->appendRow(item_a2);
        item_axis->appendRow(item_a3);
        item_axis->appendRow(item_a4);
        item_axis->appendRow(item_a5);
        item_axis->appendRow(item_a6);
        ui->treeView_instruction_set->setModel(treemdoel_MoveRobotAxis);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析变量全局的xml文件，显示到refrobot的模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_MoveRobotAxis,all_path);
        //解析项目的xml文件，显示到refrobot模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_different_range_xml_to_refrobot_model(root_MoveRobotAxis,project_path);
        //解析程序的xml文件，显示到refrobot模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_different_range_xml_to_refrobot_model(root_MoveRobotAxis,program_path);
    }
    if(current_instruction=="CALL"){
        deal_call_instruction();//处理CALL指令的函数
    }
    if(current_instruction=="WAIT"){
        deal_wait_instruction();//处理wait指令的函数
    }
    if(current_instruction=="IF..THEN..END_IF,ELSEIF..THEN,ELSE"){
        deal_if_then_instruction();//处理IF指令的函数
    }
    if(current_instruction=="WHILE..DO..END_WHILE"){
        deal_while_instruction();//处理while指令的函数
    }
    if(current_instruction=="LOOP..DO..END_LOOP"){
        deal_loop_instruction();//处理loop指令的函数
    }
    if(current_instruction=="RUN,KILL"){
        deal_run_instruction();//处理run指令的函数
    }
    if(current_instruction=="RETURN"){
        deal_return_instruction();//处理return指令的函数
    }
    if(current_instruction=="GOTO..,IF..GOTO..,LABEL.."){
        deal_goto_instruction();//处理goto指令的函数
    }
    //接下来是设置指令组的
    if(current_instruction=="Dyn"){
        //开始设置Dyn指令
        treemodel_dyn=new QStandardItemModel(0,2,ui->page_program);
        treemodel_dyn->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_dyn->setHeaderData(1,Qt::Horizontal,"数值");
        root_dyn=treemodel_dyn->invisibleRootItem();
        QStandardItem* item_dyn=new QStandardItem("Dyn");
        root_dyn->appendRow(item_dyn);
        //插入相关的参数
        QStandardItem* item_d=new QStandardItem("dyn:DYNAMIC");
        item_dyn->appendRow(item_d);
        ui->treeView_instruction_set->setModel(treemodel_dyn);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析DYNAMIC参数到节点中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_dynamic_variable_to_dyn_model(root_dyn,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_dynamic_variable_to_dyn_model(root_dyn,project_path);
        //解析程序的xml文件，显示到dyn模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_dynamic_variable_to_dyn_model(root_dyn,program_path);

    }
    if(current_instruction=="Dynovr"){
        //开始设置Dynovr指令
        treemdoel_dynovr=new QStandardItemModel(0,2,ui->page_program);
        treemdoel_dynovr->setHeaderData(0,Qt::Horizontal,"名字");
        treemdoel_dynovr->setHeaderData(1,Qt::Horizontal,"数值");
        root_dynovr=treemdoel_dynovr->invisibleRootItem();
        QStandardItem* item_dynovr=new QStandardItem("DynOvr");
        root_dynovr->appendRow(item_dynovr);
        ui->treeView_instruction_set->setModel(treemdoel_dynovr);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析整数和浮点型变量到节点中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_IF_variable_to_dynovr_model(root_dynovr,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_IF_variable_to_dynovr_model(root_dynovr,project_path);
        //解析程序的xml文件，显示到dyn模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_IF_variable_to_dynovr_model(root_dynovr,program_path);
    }
    if(current_instruction=="Ramp"){
        //开始设置Ramp指令
        treemodel_Ramp=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Ramp->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Ramp->setHeaderData(1,Qt::Horizontal,"数值");
        root_Ramp=treemodel_Ramp->invisibleRootItem();
        QStandardItem* item_ramp=new QStandardItem("Ramp");
        root_Ramp->appendRow(item_ramp);
        QStandardItem* item_ramp_type=new QStandardItem("type:RAMPTYPE");
        item_ramp->appendRow(item_ramp_type);
        //往type中添加类型item
        QStandardItem* item_T=new QStandardItem("T");
        QStandardItem* item_S=new QStandardItem("S");
        item_ramp_type->appendRow(item_T);
        item_ramp_type->appendRow(item_S);
        ui->treeView_instruction_set->setModel(treemodel_Ramp);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
    }
    if(current_instruction=="Refsys"){
        //开始设置Refsys指令
        treemodel_Refsys=new QStandardItemModel(0,2,ui->page_program);
        treemodel_Refsys->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_Refsys->setHeaderData(1,Qt::Horizontal,"数值");
        root_Refsys=treemodel_Refsys->invisibleRootItem();
        QStandardItem* item_Refsys=new QStandardItem("RefSys");
        root_Refsys->appendRow(item_Refsys);
        QStandardItem* item_sys=new QStandardItem("refSys:REFSYS_");
        item_Refsys->appendRow(item_sys);
        ui->treeView_instruction_set->setModel(treemodel_Refsys);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析CARTREFSYS数据类型到节点中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_Variable_to_Refsys_model(root_Refsys,all_path);//解析全局xml文件
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_Variable_to_Refsys_model(root_Refsys,project_path);
        //解析程序的xml文件，显示到dyn模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_Variable_to_Refsys_model(root_Refsys,program_path);
    }
    if(current_instruction=="Tool"){
        //开始设置Tool指令
        treemodel_tool=new QStandardItemModel(0,2,ui->page_program);
        treemodel_tool->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_tool->setHeaderData(1,Qt::Horizontal,"数值");
        root_tool=treemodel_tool->invisibleRootItem();
        QStandardItem* item_tool=new QStandardItem("Tool");
        root_tool->appendRow(item_tool);
        QStandardItem* item_tool_t=new QStandardItem("tool:TOOL_");
        item_tool->appendRow(item_tool_t);
        ui->treeView_instruction_set->setModel(treemodel_tool);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析关节坐标系和世界坐标系的数据类型
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_variable_to_tool_model(root_tool,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_variable_to_tool_model(root_tool,project_path);
        //解析程序的xml文件，显示到tool模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_variable_to_tool_model(root_tool,program_path);
    }
    if(current_instruction=="...=...(赋值)"){
        deal_fuzhi_instruction();
    }
    if(current_instruction=="//...(注释)"){
        deal_zhushi_instruction();
    }
    if(current_instruction=="WaitTime"){
        //开始设置waittime指令
        treemodel_waittime=new QStandardItemModel(0,2,ui->page_program);
        treemodel_waittime->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_waittime->setHeaderData(1,Qt::Horizontal,"数值");
        root_waittime=treemodel_waittime->invisibleRootItem();
        QStandardItem* item_waittime=new QStandardItem("WaitTime");
        root_waittime->appendRow(item_waittime);
        QStandardItem* item_dint=new QStandardItem("timeMs:DINT");
        item_waittime->appendRow(item_dint);
        ui->treeView_instruction_set->setModel(treemodel_waittime);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析浮点数和整数的数据类型
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_variable_to_waittime_model(root_waittime,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_variable_to_waittime_model(root_waittime,project_path);
        //解析程序的xml文件，显示到tool模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_variable_to_waittime_model(root_waittime,program_path);
    }
    if(current_instruction=="Stop"){
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        deal_stop_instruction();
    }
    if(current_instruction=="Notice"){
        //开始设置info指令
        treemodel_info=new QStandardItemModel(0,2,ui->page_program);
        treemodel_info->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_info->setHeaderData(1,Qt::Horizontal,"数值");
        root_info=treemodel_info->invisibleRootItem();
        QStandardItem* item_info=new QStandardItem("Notice");
        root_info->appendRow(item_info);
        QStandardItem* item_str=new QStandardItem("text:STRING");
        QStandardItem* item_p1=new QStandardItem("param1:ANY(OPT)");
        QStandardItem* item_p2=new QStandardItem("param2:ANY(OPT)");
        item_info->appendRow(item_str);
        item_info->appendRow(item_p1);
        item_info->appendRow(item_p2);
        ui->treeView_instruction_set->setModel(treemodel_info);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析字符串类型的数据到模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_string_variable_to_info_model(root_info,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_string_variable_to_info_model(root_info,project_path);
        //解析程序的xml文件，显示到模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_string_variable_to_info_model(root_info,program_path);
    }
    if(current_instruction=="Warning"){
        //开始设置警告指令
        treemodel_warning=new QStandardItemModel(0,2,ui->page_program);
        treemodel_warning->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_warning->setHeaderData(1,Qt::Horizontal,"数值");
        root_warning=treemodel_warning->invisibleRootItem();
        QStandardItem* item_warning=new QStandardItem("Warning");
        root_warning->appendRow(item_warning);
        QStandardItem* item_str=new QStandardItem("text:STRING");
        QStandardItem* item_param1=new QStandardItem("param1:ANY(OPT)");
        QStandardItem* item_param2=new QStandardItem("param2:ANY(OPT)");
        item_warning->appendRow(item_str);
        item_warning->appendRow(item_param1);
        item_warning->appendRow(item_param2);
        ui->treeView_instruction_set->setModel(treemodel_warning);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析字符串类型得数据到模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_string_variable_to_info_model(root_warning,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_string_variable_to_info_model(root_warning,project_path);
        //解析程序的xml文件，显示到模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_string_variable_to_info_model(root_warning,program_path);
    }
    if(current_instruction=="Error"){
        //开始设置警告指令
        treemodel_error=new QStandardItemModel(0,2,ui->page_program);
        treemodel_error->setHeaderData(0,Qt::Horizontal,"名字");
        treemodel_error->setHeaderData(1,Qt::Horizontal,"数值");
        root_error=treemodel_error->invisibleRootItem();
        QStandardItem* item_error=new QStandardItem("Error");
        root_error->appendRow(item_error);
        QStandardItem* item_str=new QStandardItem("text:STRING");
        QStandardItem* item_param1=new QStandardItem("param1:ANY(OPT)");
        QStandardItem* item_param2=new QStandardItem("param2:ANY(OPT)");
        item_error->appendRow(item_str);
        item_error->appendRow(item_param1);
        item_error->appendRow(item_param2);
        ui->treeView_instruction_set->setModel(treemodel_error);
        ui->treeView_instruction_set->setColumnWidth(0,300);
        ui->treeView_instruction_set->setColumnWidth(1,200);
        //展开节点
        ui->treeView_instruction_set->expandToDepth(1);
        //设置指令设置界面，只有返回按钮可点击
        ui->btn_instruction_variable->setEnabled(false);
        ui->btn_instruction_shijiao->setEnabled(false);
        ui->btn_instruction_add->setEnabled(false);
        ui->btn_instruction_set->setEnabled(false);
        ui->btn_instruction_confirm_2->setEnabled(false);
        //解析字符串类型得数据到模型中
        QString all_path=root_xml_file+"//"+"all.xml";//文件保存的路径
        set_string_variable_to_info_model(root_error,all_path);
        //解析项目的xml文件，显示到模型中
        QModelIndex index=ui->treeView_project->currentIndex();
        QStandardItem* current_item=treemodel_project->itemFromIndex(index);
        QStandardItem* parent_item=current_item->parent();
        QString project_name=parent_item->text();
        QString program_name=current_item->text();
        QString project_path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
        set_string_variable_to_info_model(root_error,project_path);
        //解析程序的xml文件，显示到模型中
        QString program_path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
        set_string_variable_to_info_model(root_error,program_path);
    }
}
//处理系统功能指令组中赋值指令的函数
void Widget::deal_fuzhi_instruction(){
    //新建对话框
    QDialog* fuzhi_dialog=new QDialog(ui->page_program);
    fuzhi_dialog->setWindowTitle("用户设置");
    QFormLayout form(fuzhi_dialog);
    //创建一个label和edit
    QString tf1=QString("变量");
    QLineEdit* edit1=new QLineEdit(ui->page_program);
    form.addRow(tf1,edit1);
    //再创建一个label和edit
    QString tf2=QString("赋值");
    QLineEdit* edit2=new QLineEdit(ui->page_program);
    form.addRow(tf2,edit2);
    //添加OK和CANCLE按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,fuzhi_dialog);
    form.addRow(&buttonBox);
    //当点击OK时
    //当点击OK时
    connect(&buttonBox,SIGNAL(accepted()),fuzhi_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),fuzhi_dialog,SLOT(reject()));
    //当点击OK时
    if(fuzhi_dialog->exec()==QDialog::Accepted){
        //获取用户输入的语句
        QString tf_name=edit1->text();
        QString tf_value=edit2->text();
        QString name=tf_name+"="+tf_value;
        //将内容插入到program_edit当中
        insert_instruction_to_program_edit(name);
        //返回程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }
}
//处理系统功能指令组中注释指令的函数
void Widget::deal_zhushi_instruction(){
    //注释所在行内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.insertText("//");
    //跳转到程序管理界面
    int indexpage=0;
    switch_page_program(indexpage);
}
//解析相应的数据类型，显示到ramp模型当中
void Widget::set_IF_variable_to_Ramp_model(QStandardItem *root, QString path){
    //解析xml文件
    TiXmlDocument* doc=new TiXmlDocument;
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件成功";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="INTEGER"|type_attribute[name]=="FLOAT"){
                QStandardItem* item_parameter=new QStandardItem("倾斜参数");
                QStandardItem* item_ramp=root->child(0,0)->child(1,0);
                item_ramp->appendRow(item_parameter);
                //然后将对应的参数数值插入到变量中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_parameter->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_parameter->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//解析相应的数据类型，显示到Notice模型节点中
void Widget::set_string_variable_to_info_model(QStandardItem *root, QString path){
    //解析xml文件
    TiXmlDocument* doc=new TiXmlDocument;
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件成功";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="STRING"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(0,0);
                item_dyn->appendRow(item_name);
                //然后将DYNAMIC变量插入到对应参数变量中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//处理stop指令的函数
void Widget::deal_stop_instruction(){
    //开始设置stoprobot指令
    treemodel_stop=new QStandardItemModel(0,2,ui->page_program);
    treemodel_stop->setHeaderData(0,Qt::Horizontal,"名字");
    treemodel_stop->setHeaderData(1,Qt::Horizontal,"数值");
    root_stop=treemodel_stop->invisibleRootItem();//设置根节点
    QStandardItem* item_stoprobot=new QStandardItem("Stop");
    root_stop->appendRow(item_stoprobot);
    QStandardItem* item_mode=new QStandardItem("StopMode");
    item_stoprobot->appendRow(item_mode);
    //插入参数信息
    QStandardItem* item_H=new QStandardItem("HARDSTOP");
    QStandardItem* item_S=new QStandardItem("SOFTSTOP");
    item_mode->appendRow(item_H);
    item_mode->appendRow(item_S);
    ui->treeView_instruction_set->setModel(treemodel_stop);//在树中显示Model
    ui->treeView_instruction_set->setColumnWidth(0,300);
    ui->treeView_instruction_set->setColumnWidth(1,200);
    //展开节点
    ui->treeView_instruction_set->expandToDepth(1);
    //跳转到指令设置界面
    int indexpage=2;
    switch_page_program(indexpage);
}
//解析相应的数据类型到waittime的模型节点中
void Widget::set_variable_to_waittime_model(QStandardItem *root, QString path){
    //解析xml文件
    TiXmlDocument* doc=new TiXmlDocument;
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件成功";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="INTEGER"|type_attribute[name]=="FLOAT"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(0,0);
                item_dyn->appendRow(item_name);
                //然后将DYNAMIC变量插入到对应参数变量中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//解析相应的数据类型到TOOL模型节点中
void Widget::set_variable_to_tool_model(QStandardItem *root, QString path){
    //解析xml文件
    TiXmlDocument* doc=new TiXmlDocument;
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件成功";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="TOOL"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(0,0);
                item_dyn->appendRow(item_name);
                //然后将DYNAMIC变量插入到对应参数变量中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//解析xml文件，显示到Refsys模型上
void Widget::set_Variable_to_Refsys_model(QStandardItem *root, QString path){
    //解析xml文件
    TiXmlDocument* doc=new TiXmlDocument;
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件成功";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="CARTREFSYS"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(0,0);
                item_dyn->appendRow(item_name);
                //然后将DYNAMIC变量插入到对应参数变量中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//解析xml文件，显示到dynove模型上
void Widget::set_IF_variable_to_dynovr_model(QStandardItem *root, QString path){
    //解析xml文件
    TiXmlDocument* doc=new TiXmlDocument;
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件成功";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="INTEGER"|type_attribute[name]=="FLOAT"){
                QStandardItem* item_name=new QStandardItem("ovr:PERCENT");
                QStandardItem* item_dyn=root->child(0,0);
                item_dyn->appendRow(item_name);
                //然后将DYNAMIC变量插入到对应参数变量中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//解析xml文件，显示到dyn模型上
void Widget::set_dynamic_variable_to_dyn_model(QStandardItem *root, QString path){
    //解析xml文件
    TiXmlDocument* doc=new TiXmlDocument;
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件成功";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="DYNAMIC"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(0,0);
                item_dyn->appendRow(item_name);
                //然后将DYNAMIC变量插入到对应参数变量中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//解析变量xml文件，显示到refrobotAxis的模型中
void Widget::set_different_range_xml_to_refrobotAxis_model(QStandardItem *root, QString path){
    //解析所有的xml文件
    TiXmlDocument* doc=new TiXmlDocument;//创建一个xml文档
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件失败";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            if(type_attribute[name]=="DYNAMIC"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(0,0);
                item_dyn->appendRow(item_name);
                //然后将dyn变量对应的参数变量插入对应参数变量节点中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }

            }
        }
    }
}
//解析变量xml文件，显示到refrobot的模型中
void Widget::set_different_range_xml_to_refrobot_model(QStandardItem *root, QString path){
    //解析所有的xml文件
    TiXmlDocument* doc=new TiXmlDocument;//创建一个xml文档
    //获取对应的xml文档
    if(doc->LoadFile(path.toStdString().c_str())){
        qDebug()<<"解析xml文件成功";
    }
    else{
        qDebug()<<"解析xml文件失败";
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        qDebug()<<"根节点不存在";
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //先插入整数和浮点变量
            if(type_attribute[name]=="INTEGER"|type_attribute[name]=="FLOAT"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_IF=root->child(0,0)->child(1,0);
                item_IF->appendRow(item_name);
                //然后将整数或者浮点变量插入到对应参数变量变量
                int count=0;//插入参数值时候的标志
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }

            }
            if(type_attribute[name]=="DYNAMIC"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(2,0);
                item_dyn->appendRow(item_name);
                //然后将dyn变量对应的参数变量插入对应参数变量节点中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }

            }
        }
    }
}
//解析xml文件，解析信息写到指令设置界面
void Widget::set_xml_to_PTP_Lin_Circ(QStandardItem* root){
    //需要解析所有的xml文件，
    TiXmlDocument* doc=new TiXmlDocument;//创建一个xml文档
    //获取对应的xml文档
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* current_item=treemodel_project->itemFromIndex(index);
    QStandardItem* parent_item=current_item->parent();
    QString program_name=current_item->text();
    QString project_name=parent_item->text();
    QString path=root_xml_file+"//"+project_name+"//"+program_name+".xml";
    if(doc->LoadFile(path.toStdString().c_str())){
        cout<<"load file success"<<endl;
    }
    else{
        cout<<"load file failed"<<endl;
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        cout<<"no root"<<endl;
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //将pos变量名插入模型中
            if(type_attribute[name]=="AXISPOS"|type_attribute[name]=="CARTPOS"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_pos=root->child(0,0)->child(0,0);
                item_pos->appendRow(item_name);
                //然后将pos变量对应的参数变量插入对应参数变量节点中
                int count=0;//插入参数值时候的标志
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
            //插入的是dyn参数
            if(type_attribute[name]=="DYNAMIC"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(1,0);
                item_dyn->appendRow(item_name);
                //然后将dyn变量对应的参数变量插入对应参数变量节点中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
        }
    }
}
//解析项目作用域节点到对应的模型中
void Widget::set_project_range_xml_to_PTP_Lin_Circ(QStandardItem *root){
    //需要解析所有的xml文件，
    TiXmlDocument* doc=new TiXmlDocument;//创建一个xml文档
    //获取对应的xml文档
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* current_item=treemodel_project->itemFromIndex(index);
    QStandardItem* parent_item=current_item->parent();
    QString project_name=parent_item->text();
    QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
    if(doc->LoadFile(path.toStdString().c_str())){
        cout<<"load file success"<<endl;
    }
    else{
        cout<<"load file failed"<<endl;
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        cout<<"no root"<<endl;
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //将pos变量名插入模型中
            if(type_attribute[name]=="AXISPOS"|type_attribute[name]=="CARTPOS"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_pos=root->child(0,0)->child(0,0);
                item_pos->appendRow(item_name);
                //然后将pos变量对应的参数变量插入对应参数变量节点中
                int count=0;//插入参数值时候的标志
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
            //插入的是dyn参数
            if(type_attribute[name]=="DYNAMIC"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(1,0);
                item_dyn->appendRow(item_name);
                //然后将dyn变量对应的参数变量插入对应参数变量节点中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }

            }

        }
    }
}
//解析全局作用域节点到对应的模型中
void Widget::set_all_range_xml_to_PTP_Lin_Circ(QStandardItem *root){
    //需要解析所有的xml文件，
    TiXmlDocument* doc=new TiXmlDocument;//创建一个xml文档
    //获取对应的xml文档
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* current_item=treemodel_project->itemFromIndex(index);
    QStandardItem* parent_item=current_item->parent();
    QString project_name=parent_item->text();
    QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
    if(doc->LoadFile(path.toStdString().c_str())){
        cout<<"load file success"<<endl;
    }
    else{
        cout<<"load file failed"<<endl;
    }
    //首先获取根节点，看根节点是否存在
    TiXmlElement* root_robot=doc->RootElement();
    if(root_robot==NULL){
        cout<<"no root"<<endl;
    }
    else{
        //开始解析节点下的子节点
        for(TiXmlElement* node=root_robot->FirstChildElement();node;node=node->NextSiblingElement()){
            TiXmlAttribute* node_name=node->FirstAttribute();//节点的变量名
            TiXmlAttribute* node_type=node_name->Next();//节点的数据类型
            TiXmlAttribute* node_range=node_type->Next();//节点的作用域
            QString name=QString::fromStdString(node_name->Value());
            type_attribute.insert(name,QString::fromStdString(node_type->Value()));//将变量名和数据类型插入容器中
            type_range.insert(name,QString::fromStdString(node_range->Value()));//将变量名和作用域插入容器中
            //将pos变量名插入模型中
            if(type_attribute[name]=="AXISPOS"|type_attribute[name]=="CARTPOS"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_pos=root->child(0,0)->child(0,0);
                item_pos->appendRow(item_name);
                //然后将pos变量对应的参数变量插入对应参数变量节点中
                int count=0;//插入参数值时候的标志
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }
            }
            //插入的是dyn参数
            if(type_attribute[name]=="DYNAMIC"){
                QStandardItem* item_name=new QStandardItem(QString::fromStdString(node_name->Value()));
                QStandardItem* item_dyn=root->child(0,0)->child(1,0);
                item_dyn->appendRow(item_name);
                //然后将dyn变量对应的参数变量插入对应参数变量节点中
                int count=0;
                for(TiXmlElement* node_value=node->FirstChildElement();node_value;node_value=node_value->NextSiblingElement()){
                    //获取每个参数节点的值
                    string value=node_value->GetText();
                    QStandardItem* item_value=new QStandardItem(QString::fromStdString(value));
                    //获取每个参数节点的名称
                    TiXmlAttribute* value_name=node_value->FirstAttribute();
                    string name_value=value_name->Value();
                    QStandardItem* item_value_name=new QStandardItem(QString::fromStdString(name_value));
                    //先插入参数节点的名称
                    item_name->appendRow(item_value_name);//插入名称到变量节点中
                    //接下来将参数值的item插入到对应模型的item位置中
                    item_name->setChild(count,1,item_value);
                    count++;
                }

            }

        }
    }
}

//处理stoprobot指令的函数
void Widget::deal_StopRobot_instruction(){
    //开始设置stoprobot指令
    treemodel_stoprobot=new QStandardItemModel(0,2,ui->page_program);
    treemodel_stoprobot->setHeaderData(0,Qt::Horizontal,"名字");
    treemodel_stoprobot->setHeaderData(1,Qt::Horizontal,"数值");
    root_stoprobot=treemodel_stoprobot->invisibleRootItem();//设置根节点
    QStandardItem* item_stoprobot=new QStandardItem("StopRobot");
    root_stoprobot->appendRow(item_stoprobot);
    QStandardItem* item_mode=new QStandardItem("StopMode");
    item_stoprobot->appendRow(item_mode);
    //插入参数信息
    QStandardItem* item_H=new QStandardItem("HARDSTOP");
    QStandardItem* item_S=new QStandardItem("SOFTSTOP");
    item_mode->appendRow(item_H);
    item_mode->appendRow(item_S);
    ui->treeView_instruction_set->setModel(treemodel_stoprobot);//在树中显示Model
    ui->treeView_instruction_set->setColumnWidth(0,300);
    ui->treeView_instruction_set->setColumnWidth(1,200);
    //展开节点
    ui->treeView_instruction_set->expandToDepth(1);
    //跳转到指令设置界面
    int indexpage=2;
    switch_page_program(indexpage);
}
//处理CALL指令的函数
void Widget::deal_call_instruction(){
    //新建对话框
    QDialog* CALL_dialog=new QDialog(ui->page_program);
    CALL_dialog->setWindowTitle("用户设置:");
    QFormLayout form(CALL_dialog);

    //输入变量的类型
    QString Coordiante=QString("调用程序:");
    QComboBox* combox=new QComboBox(CALL_dialog);//创建一个下拉框
    //开始遍历对应的文件路径，并且将程序名称写到combox控件上
    //首先获取当前文件路径下的目录路径
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* item=treemodel_project->itemFromIndex(index);
    QStandardItem* item_parent=item->parent();
    QString project_name=item_parent->text();
    QString path=root_prog_file+"//"+project_name;//项目路径
    QDir dir_file(path);
    dir_file.setFilter(QDir::Files);//过滤配置文件
    QFileInfoList list_file=dir_file.entryInfoList();
    QStringList filelist_file;
    for(int i=0;i<list_file.count();i++){
        QString filename_name=list_file.at(i).fileName();
        int len=filename_name.length();
        QString temp=filename_name.left(len-5);
        QString name=temp+"()";
        combox->addItem(name);//将文件名插入combox控件中
    }
    form.addRow(Coordiante,combox);

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,CALL_dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),CALL_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),CALL_dialog,SLOT(reject()));

    //当点击OK按钮时
    if(CALL_dialog->exec()==QDialog::Accepted){
        // 获取combox当前内容
        QString pro_name=combox->currentText();
        QString name="CALL "+pro_name;
        //将指令插入program_edit当中
        insert_instruction_to_program_edit(name);
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }

}
//处理wait指令的函数
void Widget::deal_wait_instruction(){
    //新建一个对话框
    QDialog* WAIT_dialog=new QDialog(ui->page_program);
    WAIT_dialog->setWindowTitle("用户设置:");
    QFormLayout form(WAIT_dialog);
    //创建一个label和combox
    QString tf=QString("条件判断:");
    QLineEdit* edit=new QLineEdit(ui->page_program);
    form.addRow(tf,edit);

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,WAIT_dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),WAIT_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),WAIT_dialog,SLOT(reject()));

    //当点击OK按钮时
    if(WAIT_dialog->exec()==QDialog::Accepted){
        //获取用户输入的语句
        QString tf_name=edit->text();
        QString name="WAIT "+tf_name;
        //将内容插入programdit当中
        insert_instruction_to_program_edit(name);
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }

}
//处理IF指令的函数
void Widget::deal_if_then_instruction(){
    //创建一个窗口，选择IF指令的类型
    QDialog* choose_dialog=new QDialog(ui->page_program);
    choose_dialog->setWindowTitle("用户设置:");
    QFormLayout form(choose_dialog);
    //创建里面的控件
    QString type=QString("条件分支选择:");
    QComboBox* combox=new QComboBox(choose_dialog);
    combox->addItem("IF..THEN..");
    combox->addItem("ELSIF..THEN..");
    combox->addItem("ELSE");
    form.addRow(type,combox);

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,choose_dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),choose_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),choose_dialog,SLOT(reject()));

    //当点击OK按钮时
    if(choose_dialog->exec()==QDialog::Accepted){
        //获取combox的内容
        QString type_name=combox->currentText();
        if(type_name=="IF..THEN.."){
            //进入当指令的设置界面
            insert_instruction_to_program_edit("END_IF");
            deal_different_if_then_instruction("IF","THEN");
        }
        if(type_name=="ELSIF..THEN.."){
            deal_different_if_then_instruction("ELSEIF","THEN");
        }
        if(type_name=="ELSE"){
            //直接将指令插入program_edit当中
            QString name="ELSE";
            insert_instruction_to_program_edit(name);
            //返回到程序编辑界面
            int indexpage=0;
            switch_page_program(indexpage);
        }
    }
}
//处理不同if指令的设置函数
void Widget::deal_different_if_then_instruction(QString a, QString b){
    //首先创建一个对话框
    QDialog* dialog=new QDialog(ui->page_program);
    dialog->setWindowTitle("用户设置:");
    QFormLayout form(dialog);
    //创建label和edit
    QString condition_name=QString("条件:");
    QLineEdit* condition_edit=new QLineEdit(dialog);
    form.addRow(condition_name,condition_edit);

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),dialog,SLOT(reject()));

    //当点击OK按钮时
    if(dialog->exec()==QDialog::Accepted){
        QString name=a+" "+condition_edit->text()+" "+b;
        //将内容插入program_edit中
        insert_instruction_to_program_edit(name);
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }

}
//处理while指令的函数
void Widget::deal_while_instruction(){
    //新建一个对话框
    QDialog* while_dialog=new QDialog(ui->page_program);
    while_dialog->setWindowTitle("用户设置:");
    QFormLayout form(while_dialog);
    //创建一个label和lineedit
    QString tf=QString("条件判断:");
    QLineEdit* edit=new QLineEdit(ui->page_program);
    form.addRow(tf,edit);

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,while_dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),while_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),while_dialog,SLOT(reject()));

    //当点击OK按钮时
    if(while_dialog->exec()==QDialog::Accepted){
        QString tf_name=edit->text();
        QString name="WHILE "+tf_name+" DO";
        insert_instruction_to_program_edit("END_WHILE");
        insert_instruction_to_program_edit(name);
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }
}
//处理loop指令的函数
void Widget::deal_loop_instruction(){
    //创建一个对话框
    QDialog* LOOP_dialog=new QDialog(ui->page_program);
    LOOP_dialog->setWindowTitle("用户设置:");
    QFormLayout form(LOOP_dialog);
    //创建一个label和edit
    QString loop_value=QString("循环次数:");
    QLineEdit* loop_edit=new QLineEdit(LOOP_dialog);
    form.addRow(loop_value,loop_edit);//布局一下

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,LOOP_dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),LOOP_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),LOOP_dialog,SLOT(reject()));

    //当点击OK按钮时
    if(LOOP_dialog->exec()==QDialog::Accepted){
        QString loop_name=loop_edit->text();
        QString name="LOOP "+loop_name+" DO";
        //将内容插入programdit当中
        insert_instruction_to_program_edit("END_LOOP");
        insert_instruction_to_program_edit(name);
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }
}
//处理run指令的函数
void Widget::deal_run_instruction(){
    //新建对话框
    QDialog* run_dialog=new QDialog(ui->page_program);
    run_dialog->setWindowTitle("用户设置:");
    QFormLayout form(run_dialog);
    //输入变量的类型
    QString Coordinate=QString("选择程序:");
    QComboBox* combox=new QComboBox(run_dialog);//创建一个下拉框
    //开始遍历对应的文件路径，并且将程序名称写到combox控件上
    //首先获取当前文件路径下的目录路径
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* item=treemodel_project->itemFromIndex(index);
    QStandardItem* item_parent=item->parent();
    QString project_name=item_parent->text();
    QString path=root_prog_file+"//"+project_name;//项目路径
    QDir dir_file(path);
    dir_file.setFilter(QDir::Files);//过滤配置文件
    QFileInfoList list_file=dir_file.entryInfoList();
    QStringList filelist_file;
    for(int i=0;i<list_file.count();i++){
        QString filename_name=list_file.at(i).fileName();
        int len=filename_name.length();
        QString temp=filename_name.left(len-5);
        QString name=temp;
        combox->addItem(name);//将文件名插入combox控件中
    }
    form.addRow(Coordinate,combox);


    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,run_dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),run_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),run_dialog,SLOT(reject()));

    //当点击OK按钮时
    if(run_dialog->exec()==QDialog::Accepted){
        // 获取combox当前内容
        QString pro_name=combox->currentText();
        QString name_run="RUN "+pro_name+"()";
        QString name_kill="KILL "+pro_name+"()";
        //将指令插入program_edit当中
        insert_instruction_to_program_edit(name_kill);
        insert_instruction_to_program_edit(name_run);
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }


}
//处理return指令的函数
void Widget::deal_return_instruction(){
    insert_instruction_to_program_edit("RETURN");
    //返回到程序编辑界面
    int indexpage=0;
    switch_page_program(indexpage);
}
//处理goto指令的函数
void Widget::deal_goto_instruction(){
    //创建一个对话框
    QDialog* goto_dialog=new QDialog(ui->page_program);
    goto_dialog->setWindowTitle("用户设置:");
    QFormLayout form(goto_dialog);
    //创建一个label和combox
    QString tf=QString("GOTO分支:");
    QComboBox* combox=new QComboBox(goto_dialog);//创建一下拉框
    //往goto中插入指令选项
    combox->addItem("GOTO..");
    combox->addItem("IF..GOTO..");
    combox->addItem("LABEL..");
    form.addRow(tf,combox);

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,goto_dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),goto_dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),goto_dialog,SLOT(reject()));

    //当点击OK按钮时
    if(goto_dialog->exec()==QDialog::Accepted){
        QString tf_name=combox->currentText();
        if(tf_name=="GOTO.."){
            QString temp1="GOTO ";
            QString temp2=" LABEL";
            QString name=temp1+temp2;
            insert_instruction_to_program_edit(name);
        }
        if(tf_name=="IF..GOTO.."){
            QString a="IF";
            QString b="GOTO LABEL";
            deal_different_if_then_instruction(a,b);
        }
        if(tf_name=="LABEL.."){
            deal_GOTO_LABEL_instruction();//处理该函数的指令
        }
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }

}
//程序管理模块:LABEL指令的函数
void Widget::deal_GOTO_LABEL_instruction(){
    //首先创建一个对话框
    QDialog* dialog=new QDialog(ui->page_program);
    dialog->setWindowTitle("用户设置:");
    QFormLayout form(dialog);
    //创建label和edit
    QString condition_name=QString("LABEL定义:");
    QLineEdit* condition_edit=new QLineEdit(dialog);
    form.addRow(condition_name,condition_edit);

    //添加OK和Cancle按钮
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox,SIGNAL(accepted()),dialog,SLOT(accept()));
    connect(&buttonBox,SIGNAL(rejected()),dialog,SLOT(reject()));

    //当点击OK按钮时
    if(dialog->exec()==QDialog::Accepted){
        QString name2=condition_edit->text();
        insert_instruction_to_program_edit(name2);
        QString name="LABEL: ";
        //将内容插入program_edit中
        insert_instruction_to_program_edit(name);
        //返回到程序编辑界面
        int indexpage=0;
        switch_page_program(indexpage);
    }
}
//程序管理模块：指令修改按钮，将修改得指令替换已有得指令
void Widget::correct_instruction_to_program_edit(QString name){
    //首先获取光标所在行的内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    //将所选行的内容删除
    cur.removeSelectedText();
    //然后将修改得指令插入该行
    ui->program_textEdit->textCursor().insertText(name);
    //开始将指令写入文件中
    //首先获取当前所加载文件的程序名称
    QString instruction_name=ui->treeView_instruction_set->currentIndex().data().toString();
    QModelIndex index_program=ui->treeView_project->currentIndex();
    QStandardItem* item_program=treemodel_project->itemFromIndex(index_program);//获取所加载程序的item
    QStandardItem* item_project=item_program->parent();//加载程序的item的父item
    QString program_name=item_program->text();
    QString project_name=item_project->text();
    QString file_name=root_prog_file+"//"+project_name+"//"+program_name+".prog";//当前所加载程序的路径名称

    QFile file(file_name);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::information(this,"错误信息","当前程序无法保存");
    }
    else{
        //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
        //若当前选中文本，需要先将文本移动到文末
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::End);
        cur.select(QTextCursor::BlockUnderCursor);
        cur.removeSelectedText();


        QTextStream out(&file);//分行写入文件
        out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
        out<<"{"<<"\n";
        out<<ui->program_textEdit->toPlainText()<<"\n";
        out<<"}";

        //写完之后，在program_edit末尾添加一行<<END_OFF>>
        ui->program_textEdit->append("<<END_OFF>>");
    }

}
//将所设置的流程控制指令插入program_edit当中，并保存
void Widget::insert_instruction_to_program_edit(QString instruction_name){
    //获取当前插入的指令名称
    QModelIndex index_program=ui->treeView_project->currentIndex();
    QStandardItem* item_program=treemodel_project->itemFromIndex(index_program);//获取所加载程序的item
    QStandardItem* item_project=item_program->parent();//过去的加载程序的item的父item
    QString program_name=item_program->text();
    QString project_name=item_project->text();
    QString file_name=root_prog_file+"//"+project_name+"//"+program_name+".prog";//当前所加载程序的路径名称
    //需要在所选中行的上一行添加指令
    if(line_program_edit==0){
        //如果文本中只有一行，需要作一定处理
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::StartOfLine);
        ui->program_textEdit->setTextCursor(cur);
        ui->program_textEdit->textCursor().insertText("\n");
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(line_program_edit-1);
        ui->program_textEdit->setTextCursor(QTextCursor(block));//将行数往上移了一行
        //开始插入指令
        ui->program_textEdit->textCursor().insertText(instruction_name);

    }
    else{
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(line_program_edit-1);
        ui->program_textEdit->setTextCursor(QTextCursor(block));//将行数往上移了一行
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::EndOfLine);
        ui->program_textEdit->setTextCursor(cur);
        ui->program_textEdit->textCursor().insertText("\n");
        ui->program_textEdit->textCursor().insertText(instruction_name);

    }
    //开始将指令写入文件中
    QFile file(file_name);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::information(this,"错误信息","当前程序无法保存");
    }
    else{
        //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
        //若当前选中文本，需要先将文本移动到文末
        int temp=line_program_edit;//保存文本当前的行数
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::End);
        cur.select(QTextCursor::BlockUnderCursor);
        cur.removeSelectedText();
        //删除完行数之后，将光标移动到刚指定的行上面
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
        ui->program_textEdit->setTextCursor(QTextCursor(block));

        QTextStream out(&file);//分行写入文件
        out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
        out<<"{"<<"\n";
        out<<ui->program_textEdit->toPlainText()<<"\n";
        out<<"}";

        //写完之后，在program_edit末尾添加一行<<END_OFF>>
        ui->program_textEdit->append("<<END_OFF>>");
    }
}

//程序管理模块：布局指令设置界面的函数
void Widget::set_instruction_set(){
    //为程序添加复制、粘贴、剪切、删除控件
    QMenu* menu_edit=new QMenu(ui->page_program);
    ui->btn_program_edit_2->setMenu(menu_edit);
    //设置菜单格式
    QAction* edit_copy=new QAction("复制",ui->page_program);
    QAction* edit_paste=new QAction("粘贴",ui->page_program);
    QAction* edit_cut=new QAction("剪切",ui->page_program);
    QAction* edit_delete=new QAction("删除",ui->page_program);
    QAction* edit_annotate=new QAction("注释",ui->page_program);
    QAction* edit_unannotate=new QAction("取消注释",ui->page_program);
    menu_edit->addAction(edit_copy);
    menu_edit->addAction(edit_paste);
    menu_edit->addAction(edit_cut);
    menu_edit->addAction(edit_delete);
    menu_edit->addAction(edit_annotate);
    menu_edit->addAction(edit_unannotate);
    //添加相应的信号槽
    connect(edit_copy,SIGNAL(triggered()),this,SLOT(slt_program_edit_copy()));
    connect(edit_paste,SIGNAL(triggered()),this,SLOT(slt_program_edit_paste()));
    connect(edit_cut,SIGNAL(triggered()),this,SLOT(slt_program_edit_cut()));
    connect(edit_delete,SIGNAL(triggered()),this,SLOT(slt_program_edit_delete()));
    connect(edit_annotate,SIGNAL(triggered()),this,SLOT(slt_program_edit_annotate()));
    connect(edit_unannotate,SIGNAL(triggered()),this,SLOT(slt_program_edit_unannotate()));
    //为变量按钮添加QAction:new
    QMenu* menu_variavle=new QMenu(ui->page_program);
    ui->btn_instruction_variable->setMenu(menu_variavle);//放到变量按钮中
    //设置菜单格式
    QAction* instruction_variable_new=new QAction("新建",ui->page_program);
    menu_variavle->addAction(instruction_variable_new);
    //新建选项的信号槽
    connect(instruction_variable_new,SIGNAL(triggered()),this,SLOT(slt_new_Variable()));
}
//程序的复制
void Widget::slt_program_edit_copy(){
    //首先获取光标所在行的内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    program_edit_copy_cut_text=cur.selectedText();
}
//程序的粘贴
void Widget::slt_program_edit_paste(){
    //在当前光标的上面一行插入指令
    if(line_program_edit==0){
        //如果文本中只有一行，需要作一定处理
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::StartOfLine);
        ui->program_textEdit->setTextCursor(cur);
        ui->program_textEdit->textCursor().insertText("\n");
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(line_program_edit-1);
        ui->program_textEdit->setTextCursor(QTextCursor(block));//将行数往上移了一行
        //开始插入指令
        ui->program_textEdit->textCursor().insertText(program_edit_copy_cut_text);
    }
    else{
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(line_program_edit-1);
        ui->program_textEdit->setTextCursor(QTextCursor(block));//将行数上移一行
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::EndOfLine);
        ui->program_textEdit->setTextCursor(cur);
        ui->program_textEdit->textCursor().insertText("\n");
        ui->program_textEdit->textCursor().insertText(program_edit_copy_cut_text);
    }
    //开始将指令写入文件中
    save_program_to_prog_file();
}
//程序的剪切
void Widget::slt_program_edit_cut(){
    //首先获取光标所在行的内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    program_edit_copy_cut_text=cur.selectedText();
    //将所选行的内容删除
    cur.removeSelectedText();
    cur.deleteChar();
}
//程序的删除
void Widget::slt_program_edit_delete(){
    //首先获取光标所在行的内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    //将所选行的内容删除
    cur.removeSelectedText();
    cur.deleteChar();
    //开始将指令写入文件中
    //首先获取当前所加载文件的程序名称
    QString instruction_name=ui->treeView_instruction_set->currentIndex().data().toString();
    QModelIndex index_program=ui->treeView_project->currentIndex();
    QStandardItem* item_program=treemodel_project->itemFromIndex(index_program);//获取所加载程序的item
    QStandardItem* item_project=item_program->parent();//加载程序的item的父item
    QString program_name=item_program->text();
    QString project_name=item_project->text();
    QString file_name=root_prog_file+"//"+project_name+"//"+program_name+".prog";//当前所加载程序的路径名称

    QFile file(file_name);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::information(this,"错误信息","当前程序无法保存");
    }
    else{
        //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
        //若当前选中文本，需要先将文本移动到文末
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::End);
        cur.select(QTextCursor::BlockUnderCursor);
        cur.removeSelectedText();


        QTextStream out(&file);//分行写入文件
        out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
        out<<"{"<<"\n";
        out<<ui->program_textEdit->toPlainText()<<"\n";
        out<<"}";

        //写完之后，在program_edit末尾添加一行<<END_OFF>>
        ui->program_textEdit->append("<<END_OFF>>");
    }

}
//程序的注释
void Widget::slt_program_edit_annotate(){
    //首先获取光标
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.insertText("//");
    //将指令写入相应的文件中
    save_program_to_prog_file();
}
//程序的取消注释
void Widget::slt_program_edit_unannotate(){
    //首先获取光标
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.movePosition(QTextCursor::Right);
    cur.deleteChar();
    cur.movePosition(QTextCursor::Left);
    cur.deleteChar();
    //将指令写入相应的文件中
    save_program_to_prog_file();
}
//将peogramme_edit上的内容写到对应的加载程序文件中
void Widget::save_program_to_prog_file(){
    //首先获取当前所加载文件的程序名称
    QString instruction_name=ui->treeView_instruction_set->currentIndex().data().toString();
    QModelIndex index_program=ui->treeView_project->currentIndex();
    QStandardItem* item_program=treemodel_project->itemFromIndex(index_program);//获取所加载程序的item
    QStandardItem* item_project=item_program->parent();//加载程序的item的父item
    QString program_name=item_program->text();
    QString project_name=item_project->text();
    QString file_name=root_prog_file+"//"+project_name+"//"+program_name+".prog";//当前所加载程序的路径名称

    QFile file(file_name);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::information(this,"错误信息","当前程序无法保存");
    }
    else{
        //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
        //若当前选中文本，需要先将文本移动到文末
        int temp=line_program_edit;//保存文本当前的行数
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::End);
        cur.select(QTextCursor::BlockUnderCursor);
        cur.removeSelectedText();
        //删除完行数之后，将光标移动到刚指定的行上面
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
        ui->program_textEdit->setTextCursor(QTextCursor(block));

        QTextStream out(&file);//分行写入文件
        out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
        out<<"{"<<"\n";
        out<<ui->program_textEdit->toPlainText()<<"\n";
        out<<"}";

        //写完之后，在program_edit末尾添加一行<<END_OFF>>
        ui->program_textEdit->append("<<END_OFF>>");
    }
}
//程序管理模块，指令选择界面，点击不同的item，按钮的点击权限设置
void Widget::slot_treeview_instruction_choose_CurrentChanged(const QModelIndex& t, const QModelIndex& previous){
    QStandardItem* item1=treemodel_instruction_choose->itemFromIndex(t);
    QStandardItem* item2=treemodel_instruction_choose->itemFromIndex(previous);
    if(item1->parent()!=NULL&&item1->child(0)==NULL){
        //此时选择的是具体的指令，可以点击确定按钮
        ui->btn_instruction_confirm->setEnabled(true);
    }
    else{
        //此时点击的霏具体指令，不可以点击确定按钮
        ui->btn_instruction_confirm->setEnabled(false);
    }
}
//程序管理模块：指令修改界面，点击不同的item，按钮的点击权限设置
void Widget::slot_treeview_instruction_program_correct_CurrentChanged(const QModelIndex& t, const QModelIndex& previous){
    //获取光标所在行得内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    QString instruction=cur.selectedText();
    QString judge=instruction.left(instruction.indexOf("(",1));
    //根据不同得指令，进入到不同得指令设置界面
    if(judge=="PTP"|judge=="Lin"|judge=="PTPRel"|judge=="LinRel"){
        QStandardItem* item1;
        if(judge=="PTP"){
            item1=treemodel_PTP->itemFromIndex(t);
        }
        if(judge=="Lin"){
            item1=treemodel_Lin->itemFromIndex(t);
        }
        if(judge=="PTPRel"){
            item1=treemodel_PTPrel->itemFromIndex(t);
        }
        if(judge=="LinRel"){
            item1=treemodel_Linrel->itemFromIndex(t);
        }
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="pos:POSITION_"|item1->text()=="dyn:DYNAMIC_(OPT)"|item1->text()=="ovl:OVERLAP_(OPT)"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==6|item1->rowCount()==12|item1->rowCount()==7){
            ui->btn_program_edit_shijiao->setEnabled(false);
            //示教按钮只对AXISPOS和CARTPOS两类位置变量有效
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_program_edit_shijiao->setEnabled(true);
            }
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="Circ"){
        QStandardItem* item1=treemodel_Circ->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="circPos:POSITION"|item1->text()=="dyn_DYNAMIC_(OPT)"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==6|item1->rowCount()==12|item1->rowCount()==7){
            ui->btn_program_edit_shijiao->setEnabled(false);
            //示教按钮只对AXISPOS和CARTPOS两类位置变量有效
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_program_edit_shijiao->setEnabled(true);
            }
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="StopRobot"){
        QStandardItem* item1=treemodel_stoprobot->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="StopMode"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->text()=="HARDSTOP"|item1->text()=="SOFTSTOP"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="RefRobotAxis"){
        QStandardItem* item1=treemodel_refrobotaxis->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="axis:ROBOTAXIS"|item1->text()=="addMoveTarget:REAL(OPT)"|item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==1|item1->rowCount()==12|item1->text()=="A1"|item1->text()=="A2"|item1->text()=="A3"|item1->text()=="A4"|item1->text()=="A5"|item1->text()=="A6"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="RefRobotAxisAsync"){
        QStandardItem* item1=treemodel_RefRobotAxisAsync->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==12){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="MoveRobotAxis"){
        QStandardItem* item1=treemdoel_MoveRobotAxis->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="axis:ROBOTAXIS"|item1->text()=="pos:REAL"|item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==1|item1->rowCount()==12|item1->text()=="A1"|item1->text()=="A2"|item1->text()=="A3"|item1->text()=="A4"|item1->text()=="A5"|item1->text()=="A6"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="Dyn"){
        QStandardItem* item1=treemodel_dyn->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="dyn:DYNAMIC"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==12){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="DynOvr"){
        QStandardItem* item1=treemdoel_dynovr->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="整数数值"|item1->text()=="浮点数数值"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
        }
        else if(item1->text()=="ovr:PERCENT"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="Ramp"){
        QStandardItem* item1=treemodel_Ramp->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="type:RAMPTYPE"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->text()=="T"|item1->text()=="S"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
    }
    if(judge=="Refsys"){
        QStandardItem* item1=treemodel_Refsys->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="refSys:REFSYS_"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==7){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="Tool"){
        QStandardItem* item1=treemodel_tool->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge&&item1->rowCount()==1){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="tool:TOOL_"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==6){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="WaitTime"){
        QStandardItem* item1=treemodel_waittime->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="timeMs:DINT"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="整数数值"|item1->child(0,0)->text()=="浮点数数值"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="Stop"){
        QStandardItem* item1=treemodel_stop->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="StopMode"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->text()=="HARDSTOP"|item1->text()=="SOFTSTOP"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
        }
    }
    if(judge=="Notice"){
        QStandardItem* item1=treemodel_info->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="text:STRING"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="字符串"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="Warning"){
        QStandardItem* item1=treemodel_warning->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="text:STRING"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="字符串"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
    if(judge=="Error"){
        QStandardItem* item1=treemodel_error->itemFromIndex(t);
        if(item1->text().left(judge.length())==judge){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            if(item1->text().length()>judge.length()){
                ui->btn_program_edit_confirm->setEnabled(true);
            }
            else{
                ui->btn_program_edit_confirm->setEnabled(false);
            }
        }
        else if(item1->text()=="text:STRING"){
            //此时变量按钮不可点击，其余可以
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="字符串"){
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(false);
            ui->btn_program_edit_confirm->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(true);
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_program_correct->setExpanded(index,true);
                }
                else{
                    ui->treeView_program_correct->setExpanded(index,false);
                }
            }
        }
        else{
            ui->btn_program_edit_shijiao->setEnabled(false);
            ui->btn_program_edit_add->setEnabled(false);
            ui->btn_program_edit_set->setEnabled(true);
            ui->btn_program_edit_confirm->setEnabled(false);
        }
    }
}
//程序管理模块：指令设置界面，点击不同的item，按钮的点击权限设置
void Widget::slot_treeview_instruction_set_CurrentChanged(const QModelIndex& t, const QModelIndex& previous){
    QModelIndex index_instruction_choose=ui->treeView_instruction_choose->currentIndex();
    QStandardItem* item_instruction_choose=treemodel_instruction_choose->itemFromIndex(index_instruction_choose);
    QString instruction_choose_name=item_instruction_choose->text();
    //根据在treeview_instruction_choose上所选的不同指令，做出不同的相应
    if(instruction_choose_name=="PTP"){
        QStandardItem* item1=treemodel_PTP->itemFromIndex(t);
        if(item1->text().left(3)=="PTP"){
            //选择的为PTP，此时变量、示教、添加、设置按钮不可选
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>3){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="pos:POSITION_"|item1->text()=="dyn:DYNAMIC_(OPT)"|item1->text()=="ovl:OVERLAP_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==6|item1->rowCount()==12|item1->rowCount()==7){
            //此时点击的为变量名，变量按钮、设置按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            //示教按钮只对AXISPOS和CARTPOS两类位置变量有效
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_instruction_shijiao->setEnabled(true);
            }
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            //判断变量名此时是否已经设置好值
            if(item1->child(0,1)==NULL){
                //未设置好值，不可点击添加
                ui->btn_instruction_add->setEnabled(true);
            }
            else{
                //设置好值，可以点击添加
                ui->btn_instruction_add->setEnabled(true);
            }
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Lin"){
        QStandardItem* item1=treemodel_Lin->itemFromIndex(t);
        if(item1->text().left(3)=="Lin"){
            //选择的为PTP，此时变量、示教、添加、设置按钮不可选
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>3){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="pos:POSITION_"|item1->text()=="dyn:DYNAMIC_(OPT)"|item1->text()=="ovl:OVERLAP_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==7|item1->rowCount()==12|item1->rowCount()==6){
            //此时点击的为变量名，变量按钮、设置按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            //示教按钮只对AXISPOS和CARTPOS两类位置变量有效
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_instruction_shijiao->setEnabled(true);
            }
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            //判断变量名此时是否已经设置好值
            if(item1->child(0,1)==NULL){
                //未设置好值，不可点击添加
                ui->btn_instruction_add->setEnabled(true);
            }
            else{
                //设置好值，可以点击添加
                ui->btn_instruction_add->setEnabled(true);
            }
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }

    }
    if(instruction_choose_name=="Circ"){
        QStandardItem* item1=treemodel_Circ->itemFromIndex(t);
        if(item1->text().left(4)=="Circ"){
            //选择的为PTP，此时变量、示教、添加、设置按钮不可选
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>4){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="circPos:POSITION_"|item1->text()=="dyn_DYNAMIC_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==7|item1->rowCount()==12|item1->rowCount()==6){
            //此时点击的为变量名，变量按钮、设置按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            //示教按钮只对AXISPOS和CARTPOS两类位置变量有效
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_instruction_shijiao->setEnabled(true);
            }
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            //判断变量名此时是否已经设置好值
            if(item1->child(0,1)==NULL){
                //未设置好值，不可点击添加
                ui->btn_instruction_add->setEnabled(true);
            }
            else{
                //设置好值，可以点击添加
                ui->btn_instruction_add->setEnabled(true);
            }
            //设置选中的节点展开，其余的不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="PTPRel"){
        QStandardItem* item1=treemodel_PTPrel->itemFromIndex(t);
        if(item1->text().left(6)=="PTPRel"){
            //选择的为PTPRel,此时变量、示教、添加、设置按钮不可选
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>6){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="dist:DISTANCE"|item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==6|item1->rowCount()==7|item1->rowCount()==12){
            //此时点击的为变量名，变量按钮、设置按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_instruction_shijiao->setEnabled(true);
            }
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="LinRel"){
        QStandardItem* item1=treemodel_Linrel->itemFromIndex(t);
        if(item1->text().left(6)=="LinRel"){
            //选择为LinRel,此时变量、示教、添加、设置按钮不可选
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>6){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="dist:DISTANCE_"|item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==6|item1->rowCount()==7|item1->rowCount()==12){
            //此时点击的变量名，变量按钮、设置按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_instruction_shijiao->setEnabled(true);
            }
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="StopRobot"){
        QStandardItem* item1=treemodel_stoprobot->itemFromIndex(t);
        if(item1->text().left(9)=="StopRobot"){
            //选择为LinRel,此时变量、示教、添加、设置按钮不可选
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>9){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="StopMode"){
            //此时变量按钮可以点击，其余按钮不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->text()=="HARDSTOP"|item1->text()=="SOFTSTOP"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
        }
    }
    if(instruction_choose_name=="RefRobotAxis"){
        QStandardItem* item1=treemodel_refrobotaxis->itemFromIndex(t);
        if(item1->text().left(12)=="RefRobotAxis"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>12){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="axis:ROBOTAXIS"|item1->text()=="addMoveTarget:REAL(OPT)"|item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==1|item1->rowCount()==12|item1->text()=="A1"|item1->text()=="A2"|item1->text()=="A3"|item1->text()=="A4"|item1->text()=="A5"|item1->text()=="A6"){
            //此时点击的变量名，变量按钮、设置按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            if(type_attribute[item1->text()]=="AXISPOS"|type_attribute[item1->text()]=="CARTPOS"){
                ui->btn_instruction_shijiao->setEnabled(true);
            }
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="RefRobotAxisAsync"){
        QStandardItem* item1=treemodel_RefRobotAxisAsync->itemFromIndex(t);
        if(item1->text().left(17)=="RefRobotAxisAsync"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>17){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==12){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="MoveRobotAxis"){
        QStandardItem* item1=treemdoel_MoveRobotAxis->itemFromIndex(t);
        if(item1->text().left(13)=="MoveRobotAxis"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>13){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="axis:ROBOTAXIS"|item1->text()=="pos:REAL"|item1->text()=="dyn:DYNAMIC_(OPT)"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==1|item1->rowCount()==12|item1->text()=="A1"|item1->text()=="A2"|item1->text()=="A3"|item1->text()=="A4"|item1->text()=="A5"|item1->text()=="A6"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Dyn"){
        QStandardItem* item1=treemodel_dyn->itemFromIndex(t);
        if(item1->text().left(3)=="Dyn"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>3){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="dyn:DYNAMIC"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==12){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Dynovr"){
        QStandardItem* item1=treemdoel_dynovr->itemFromIndex(t);
        if(item1->text().left(6)=="DynOvr"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>6){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="ovr:PERCENT"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Ramp"){
        QStandardItem* item1=treemodel_Ramp->itemFromIndex(t);
        if(item1->text().left(4)=="Ramp"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>4){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="type:RAMPTYPE"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->text()=="T"|item1->text()=="S"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Refsys"){
        QStandardItem* item1=treemodel_Refsys->itemFromIndex(t);
        if(item1->text().left(6)=="RefSys"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>6){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="refSys:REFSYS_"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==7){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Tool"){
        QStandardItem* item1=treemodel_tool->itemFromIndex(t);
        if(item1->text().left(4)=="Tool"&&item1->rowCount()==1){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>4){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="tool:TOOL_"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==6){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="WaitTime"){
        QStandardItem* item1=treemodel_waittime->itemFromIndex(t);
        if(item1->text().left(8)=="WaitTime"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>8){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="timeMs:DINT"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="整数数值"|item1->child(0,0)->text()=="浮点数数值"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Stop"){
        QStandardItem* item1=treemodel_stop->itemFromIndex(t);
        if(item1->text().left(4)=="Stop"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>8){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="StopMode"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->text()=="HARDSTOP"|item1->text()=="SOFTSTOP"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
        }
    }
    if(instruction_choose_name=="Notice"){
        QStandardItem* item1=treemodel_info->itemFromIndex(t);
        if(item1->text().left(6)=="Notice"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>6){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="text:STRING"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="字符串"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Warning"){
        QStandardItem* item1=treemodel_warning->itemFromIndex(t);
        if(item1->text().left(7)=="Warning"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>7){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="text:STRING"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="字符串"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
    if(instruction_choose_name=="Error"){
        QStandardItem* item1=treemodel_error->itemFromIndex(t);
        if(item1->text().left(5)=="Error"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            if(item1->text().length()>5){
                ui->btn_instruction_confirm_2->setEnabled(true);
            }
            else{
                ui->btn_instruction_confirm_2->setEnabled(false);
            }
        }
        else if(item1->text()=="text:STRING"){
            //此时变量按钮可以点击，其余不可点击
            ui->btn_instruction_variable->setEnabled(true);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
        else if(item1->rowCount()==1&&item1->child(0,0)->text()=="字符串"){
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_set->setEnabled(false);
            ui->btn_instruction_confirm_2->setEnabled(false);
            ui->btn_instruction_add->setEnabled(true);
            //设置选中的节点展开，其余不展开
            QStandardItem* item_parent=item1->parent();//获取当前所选
            vector<QStandardItem*> judge(item_parent->rowCount());
            for(int i=0;i<item_parent->rowCount();i++){
                judge[i]=item_parent->child(i,0);
                QModelIndex index=judge[i]->index();
                if(judge[i]==item1){
                    ui->treeView_instruction_set->setExpanded(index,true);
                }
                else{
                    ui->treeView_instruction_set->setExpanded(index,false);
                }
            }
        }
        else{
            //此时选择的为变量名下对应的参数，变量按钮、示教按钮、添加按钮、确定按钮不可点击
            ui->btn_instruction_variable->setEnabled(false);
            ui->btn_instruction_shijiao->setEnabled(false);
            ui->btn_instruction_add->setEnabled(false);
            ui->btn_instruction_set->setEnabled(true);
            ui->btn_instruction_confirm_2->setEnabled(false);
        }
    }
}
//程序管理模块：指令设置界面新建按钮的槽函数
void Widget::slt_new_Variable(){
    QModelIndex index_instruction_choose=ui->treeView_instruction_choose->currentIndex();
    QStandardItem* item_instruction_choose=treemodel_instruction_choose->itemFromIndex(index_instruction_choose);
    QString instruction_choose_name=item_instruction_choose->text();
    //根据在treeview_instruction_choose上所选的不同指令，做出不同的相应
    if(instruction_choose_name=="PTP"){
        //根据创建的是pos还是dyn参数进行判断
        if(ui->treeView_instruction_set->currentIndex().data().toString()=="pos:POSITION_"){
            //新建变量，输入坐标类型和变量名
            QDialog* PTP_pos_dialog=new QDialog(ui->page_program);//新建一个输入参数的对话框
            PTP_pos_dialog->setWindowTitle("pos变量设置");
            QFormLayout form(PTP_pos_dialog);
            form.addRow(new QLabel("用户输入:"));
            //变量的名称
            QString Variable_name=QString("变量名:");
            QLineEdit* lineedit=new QLineEdit(PTP_pos_dialog);
            form.addRow(Variable_name,lineedit);
            //输入变量数据类型
            QString Coordinate=QString("数据类型:");
            QComboBox* combox=new QComboBox(PTP_pos_dialog);//创建一个下拉框
            combox->addItem("AXISPOS");//关节坐标系
            combox->addItem("CARTPOS");//笛卡尔坐标系
            form.addRow(Coordinate,combox);//布局一下
            //输入变量的作用域
            QString range=QString("变量作用域:");
            QComboBox* combox_range=new QComboBox(PTP_pos_dialog);
            combox_range->addItem("全局");
            combox_range->addItem("项目");
            combox_range->addItem("程序");
            form.addRow(range,combox_range);

            //添加OK和Cancle按钮
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,PTP_pos_dialog);
            form.addRow(&buttonBox);
            connect(&buttonBox,SIGNAL(accepted()),PTP_pos_dialog,SLOT(accept()));
            connect(&buttonBox,SIGNAL(rejected()),PTP_pos_dialog,SLOT(reject()));

            QString name;
            //当点击OK按钮时
            if(PTP_pos_dialog->exec()==QDialog::Accepted){
                //读取当前用户所设置的pos变量名、数据类型和变量作用域
                QString type_name=combox->currentText();//数据类型
                QString type_range_name=combox_range->currentText();//数据作用域
                name=lineedit->text();
                type_attribute.insert(name,type_name);//将变量名和属性插入容器中
                type_range.insert(name,type_range_name);//将变量名和作用域名称插入容器中
                //将新建的变量插入模型视图中
                QStandardItem* item_variable=new QStandardItem(lineedit->text());
                QModelIndex index=ui->treeView_instruction_set->currentIndex();
                QStandardItem* item=treemodel_PTP->itemFromIndex(index);
                item->appendRow(item_variable);
                if(type_name=="AXISPOS"){
                    set_pos_AXIPOS_parameter(item_variable);//插入AXIPOS对应的指令
                }
                if(type_name=="CARTPOS"){
                    set_pos_CARTPOS_parameter(item_variable);//插入CARTPOS对应的指令
                }
                //将新建得变量，添加到xml文件中
                set_new_variable_from_instruction_set_widget_to_different_range_xml_file(item_variable,name);
                //将新建的变量，添加到var文件中
                set_new_variable_from_instruction_set_widget_to_different_range_var_file(item_variable,name);
            }
        }
        if(ui->treeView_instruction_set->currentIndex().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //此时新建的是dyn参数变量
            QDialog* PTP_dyn_dialog=new QDialog(ui->page_program);
            PTP_dyn_dialog->setWindowTitle("dyn变量设置");
            QFormLayout form(PTP_dyn_dialog);
            form.addRow(new QLabel("用户输入:"));
            //变量的名称
            QString Variable_name=QString("变量名:");
            QLineEdit* lineedit=new QLineEdit(PTP_dyn_dialog);
            form.addRow(Variable_name,lineedit);
            //输入变量的作用域
            QString range=QString("变量作用域:");
            QComboBox* combox_range=new QComboBox(PTP_dyn_dialog);
            combox_range->addItem("全局");
            combox_range->addItem("项目");
            combox_range->addItem("程序");
            form.addRow(range,combox_range);

            //添加OK和Cancle按钮
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,PTP_dyn_dialog);
            form.addRow(&buttonBox);
            connect(&buttonBox,SIGNAL(accepted()),PTP_dyn_dialog,SLOT(accept()));
            connect(&buttonBox,SIGNAL(rejected()),PTP_dyn_dialog,SLOT(reject()));

            QString name;
            //当点击OK按钮时
            if(PTP_dyn_dialog->exec()==QDialog::Accepted){
                //读取当前用户所设置的pos变量名、数据类型和变量作用域
                QString type_range_name=combox_range->currentText();//数据作用域
                name=lineedit->text();
                type_attribute.insert(name,"DYNAMIC");//将变量名和数据类型插入容器中
                type_range.insert(name,type_range_name);//将变量名和作用域名称插入容器中
                //将新建的变量插入模型视图中
                QStandardItem* item_variable=new QStandardItem(lineedit->text());
                QModelIndex index=ui->treeView_instruction_set->currentIndex();
                QStandardItem* item=treemodel_PTP->itemFromIndex(index);
                item->appendRow(item_variable);
                set_dyn_parameter(item_variable);//设置dyn的指令
                //将新建得变量，添加到xml文件中
                set_new_variable_from_instruction_set_widget_to_different_range_xml_file(item_variable,name);
                //将新建的变量，添加到var文件中
                set_new_variable_from_instruction_set_widget_to_different_range_var_file(item_variable,name);
            }         
        }

    }
    if(instruction_choose_name=="Lin"){
        //根据创建的是Pos还是dyn参数进行判断
        if(ui->treeView_instruction_set->currentIndex().data().toString()=="pos:POSITION_"){
            //新建变量，变量名
            QDialog* Lin_pos_dailog=new QDialog(ui->page_program);
            Lin_pos_dailog->setWindowTitle("pos变量设置");
            QFormLayout form(Lin_pos_dailog);
            form.addRow(new QLabel("用户输入:"));
            //变量的名称
            QString Variable_name=QString("变量名:");
            QLineEdit* lineedit=new QLineEdit(Lin_pos_dailog);
            form.addRow(Variable_name,lineedit);
            //输入变量数据类型
            QString Coordinate=QString("数据类型:");
            QComboBox* combox=new QComboBox(Lin_pos_dailog);//创建一个下拉框
            combox->addItem("AXISPOS");//关节坐标系
            combox->addItem("CARTPOS");//笛卡尔坐标系
            form.addRow(Coordinate,combox);//布局一下
            //输入变量的作用域
            QString range=QString("变量作用域:");
            QComboBox* combox_range=new QComboBox(Lin_pos_dailog);
            combox_range->addItem("全局");
            combox_range->addItem("项目");
            combox_range->addItem("程序");
            form.addRow(range,combox_range);

            //添加OK和Cancle按钮
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,Lin_pos_dailog);
            form.addRow(&buttonBox);
            connect(&buttonBox,SIGNAL(accepted()),Lin_pos_dailog,SLOT(accept()));
            connect(&buttonBox,SIGNAL(rejected()),Lin_pos_dailog,SLOT(reject()));

            QString name;
            //当点击OK按钮时
            if(Lin_pos_dailog->exec()==QDialog::Accepted){
                //读取当前用户所设置的pos变量名、数据类型和变量作用域
                QString type_name=combox->currentText();//数据类型
                QString type_range_name=combox_range->currentText();//数据作用域
                name=lineedit->text();
                type_attribute.insert(name,type_name);//将变量名和属性插入容器中
                type_range.insert(name,type_range_name);//将变量名和作用域名称插入容器中
                //将新建的变量插入模型视图中
                QStandardItem* item_variable=new QStandardItem(lineedit->text());
                QModelIndex index=ui->treeView_instruction_set->currentIndex();
                QStandardItem* item=treemodel_Lin->itemFromIndex(index);
                item->appendRow(item_variable);
                if(type_name=="AXISPOS"){
                    set_pos_AXIPOS_parameter(item_variable);//插入AXIPOS对应的指令
                }
                if(type_name=="CARTPOS"){
                    set_pos_CARTPOS_parameter(item_variable);//插入CARTPOS对应的指令
                }
                //将新建得变量，添加到xml文件中
                set_new_variable_from_instruction_set_widget_to_different_range_xml_file(item_variable,name);
                //将新建的变量，添加到var文件中
                set_new_variable_from_instruction_set_widget_to_different_range_var_file(item_variable,name);
            }
        }
        if(ui->treeView_instruction_set->currentIndex().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //此时新建的是dyn参数变量
            QDialog* Lin_dyn_dialog=new QDialog(ui->page_program);
            Lin_dyn_dialog->setWindowTitle("dyn变量设置");
            QFormLayout form(Lin_dyn_dialog);
            form.addRow(new QLabel("用户输入:"));
            //变量的名称
            QString Variable_name=QString("变量名:");
            QLineEdit* lineedit=new QLineEdit(Lin_dyn_dialog);
            form.addRow(Variable_name,lineedit);
            //输入变量的作用域
            QString range=QString("变量作用域:");
            QComboBox* combox_range=new QComboBox(Lin_dyn_dialog);
            combox_range->addItem("全局");
            combox_range->addItem("项目");
            combox_range->addItem("程序");
            form.addRow(range,combox_range);

            //添加OK和Cancle按钮
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,Lin_dyn_dialog);
            form.addRow(&buttonBox);
            connect(&buttonBox,SIGNAL(accepted()),Lin_dyn_dialog,SLOT(accept()));
            connect(&buttonBox,SIGNAL(rejected()),Lin_dyn_dialog,SLOT(reject()));

            QString name;
            //当点击OK按钮时
            if(Lin_dyn_dialog->exec()==QDialog::Accepted){
                //读取当前用户所设置的pos变量名、数据类型和变量作用域
                QString type_range_name=combox_range->currentText();//数据作用域
                name=lineedit->text();
                type_attribute.insert(name,"DYNAMIC");//将变量名和数据类型插入容器中
                type_range.insert(name,type_range_name);//将变量名和作用域名称插入容器中
                //将新建的变量插入模型视图中
                QStandardItem* item_variable=new QStandardItem(lineedit->text());
                QModelIndex index=ui->treeView_instruction_set->currentIndex();
                QStandardItem* item=treemodel_Lin->itemFromIndex(index);
                item->appendRow(item_variable);
                //在新建的变量后面插入相关的参数item
                set_dyn_parameter(item_variable);//设置dyn指令
                //将新建得变量，添加到xml文件中
                set_new_variable_from_instruction_set_widget_to_different_range_xml_file(item_variable,name);
                //将新建的变量，添加到var文件中
                set_new_variable_from_instruction_set_widget_to_different_range_var_file(item_variable,name);
            }        
        }
    }
    if(instruction_choose_name=="Circ"){
        //此时为Circ指令，新建一个Circ变量
        if(ui->treeView_instruction_set->currentIndex().data().toString()=="circPos:POSITION_"){
            //新建变量，变量名
            QDialog* Circ_pos_dialog;
            Circ_pos_dialog=new QDialog(ui->page_program);
            Circ_pos_dialog->setWindowTitle("pos变量设置");
            QFormLayout form(Circ_pos_dialog);
            form.addRow(new QLabel("用户输入:"));
            //变量的名称
            QString Variable_name=QString("变量名:");
            QLineEdit* lineedit=new QLineEdit(Circ_pos_dialog);
            form.addRow(Variable_name,lineedit);
            //输入变量数据类型
            QString Coordinate=QString("数据类型:");
            QComboBox* combox=new QComboBox(Circ_pos_dialog);//创建一个下拉框
            combox->addItem("AXISPOS");//关节坐标系
            combox->addItem("CARTPOS");//笛卡尔坐标系
            form.addRow(Coordinate,combox);//布局一下
            //输入变量的作用域
            QString range=QString("变量作用域:");
            QComboBox* combox_range=new QComboBox(Circ_pos_dialog);
            combox_range->addItem("全局");
            combox_range->addItem("项目");
            combox_range->addItem("程序");
            form.addRow(range,combox_range);

            //添加OK和Cancle按钮
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,Circ_pos_dialog);
            form.addRow(&buttonBox);
            connect(&buttonBox,SIGNAL(accepted()),Circ_pos_dialog,SLOT(accept()));
            connect(&buttonBox,SIGNAL(rejected()),Circ_pos_dialog,SLOT(reject()));

            QString name;
            //当点击OK按钮时
            if(Circ_pos_dialog->exec()==QDialog::Accepted){
                //读取当前用户所设置的pos变量名、数据类型和变量作用域
                QString type_name=combox->currentText();//数据类型
                QString type_range_name=combox_range->currentText();//数据作用域
                name=lineedit->text();
                type_attribute.insert(name,type_name);//将变量名和属性插入容器中
                type_range.insert(name,type_range_name);//将变量名和作用域名称插入容器中
                //将新建的变量插入模型视图中
                QStandardItem* item_variable=new QStandardItem(lineedit->text());
                QModelIndex index=ui->treeView_instruction_set->currentIndex();
                QStandardItem* item=treemodel_Circ->itemFromIndex(index);
                item->appendRow(item_variable);
                //在新建变量的后面插入相关的参数item
                if(type_name=="AXISPOS"){
                    set_pos_AXIPOS_parameter(item_variable);//插入AXIPOS对应的指令
                }
                if(type_name=="CARTPOS"){
                    set_pos_CARTPOS_parameter(item_variable);//插入CARTPOS对应的指令
                }
                //将新建得变量，添加到xml文件中
                set_new_variable_from_instruction_set_widget_to_different_range_xml_file(item_variable,name);
                //将新建的变量，添加到var文件中
                set_new_variable_from_instruction_set_widget_to_different_range_var_file(item_variable,name);
            }
        }
        if(ui->treeView_instruction_set->currentIndex().data().toString()=="dyn_DYNAMIC_(OPT)"){
            //此时新建的是dyn参数变量
            QDialog* Circ_dyn_dialog=new QDialog(ui->page_program);
            Circ_dyn_dialog->setWindowTitle("dyn变量设置");
            QFormLayout form(Circ_dyn_dialog);
            //变量的名称
            QString Variable_name=QString("变量名:");
            QLineEdit* lineedit=new QLineEdit(Circ_dyn_dialog);
            form.addRow(Variable_name,lineedit);
            //输入变量的作用域
            QString range=QString("变量作用域:");
            QComboBox* combox_range=new QComboBox(Circ_dyn_dialog);
            combox_range->addItem("全局");
            combox_range->addItem("项目");
            combox_range->addItem("程序");
            form.addRow(range,combox_range);

            //添加OK和Cancle按钮
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,Qt::Horizontal,Circ_dyn_dialog);
            form.addRow(&buttonBox);
            connect(&buttonBox,SIGNAL(accepted()),Circ_dyn_dialog,SLOT(accept()));
            connect(&buttonBox,SIGNAL(rejected()),Circ_dyn_dialog,SLOT(reject()));

            QString name;
            //当点击OK按钮时
            if(Circ_dyn_dialog->exec()==QDialog::Accepted){
                //读取当前用户所设置的pos变量名、数据类型和变量作用域
                QString type_range_name=combox_range->currentText();//数据作用域
                name=lineedit->text();
                type_attribute.insert(name,"DYNAMIC");//将变量名和数据类型插入容器中
                type_range.insert(name,type_range_name);//将变量名和作用域名称插入容器中
                //将新建的变量插入模型视图中
                QStandardItem* item_variable=new QStandardItem(lineedit->text());
                QModelIndex index=ui->treeView_instruction_set->currentIndex();
                QStandardItem* item=treemodel_Circ->itemFromIndex(index);
                item->appendRow(item_variable);
                //在新建的变量后面插入相关的参数item
                set_dyn_parameter(item_variable);
                //将新建得变量，添加到xml文件中
                set_new_variable_from_instruction_set_widget_to_different_range_xml_file(item_variable,name);
                //将新建的变量，添加到var文件中
                set_new_variable_from_instruction_set_widget_to_different_range_var_file(item_variable,name);
            }          
        }
    }
}
//将指令设置界面中得新建得变量添加到var文件中
void Widget::set_new_variable_from_instruction_set_widget_to_different_range_var_file(QStandardItem *item, QString name){
    //获取新建变量得数据类型
    QString type=type_attribute[name];
    //获取对应变量得数据格式
    QString insert_format=get_different_format_var_string(item,type);
    if(type_range[name]=="全局"){
        //将该变量写到all的var文件中
        //获取各个项目的路径，插入指令
        QDir dir1(root_var_file);
        dir1.setFilter(QDir::Dirs);//顾虑配置，接受文件
        QFileInfoList list1=dir1.entryInfoList();
        for(int k=0;k<list1.count();k++){
            QString filename=list1.at(k).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString program_path=root_var_file+"//"+name;
                QDir dir2(program_path);
                dir2.setFilter(QDir::Files);
                QFileInfoList list2=dir2.entryInfoList();
                for(int l=0;l<list2.count();l++){
                    QString file_name=list2.at(l).fileName();
                    //获取文件的写入路径
                    QString path=root_var_file+"//"+name+"//"+file_name;
                    //将指令插入当前的var项目中
                    insert_format_value_to_var_file(path,insert_format);
                }
            }
        }
    }
    if(type_range[name]=="项目"){
        //获取路径
        QString project_path=ui->treeView_project->currentIndex().parent().data().toString();
        QString path1=root_var_file+"//"+project_path;
        //获取各项目程序的路径
        QDir dir1(path1);
        dir1.setFilter(QDir::Files);
        QFileInfoList list1=dir1.entryInfoList();
        for(int k=0;k<list1.count();k++){
            QString filename=list1.at(k).fileName();
            if(filename=="."||filename==".."){

            }
            else{
                QString name=filename;//获取非"."文件
                QString path=root_var_file+"//"+project_path+"//"+name;
                //将指令插入当前的var项目中
                insert_format_value_to_var_file(path,insert_format);
            }
        }
    }
    if(type_range[name]=="程序"){
        //获取项目路径
        QString p1=ui->treeView_project->currentIndex().parent().data().toString();
        QString p2=ui->treeView_project->currentIndex().data().toString();
        QString path=root_var_file+"//"+p1+"//"+p2+".var";
        //将指令插入当前的var项目中
        insert_format_value_to_var_file(path,insert_format);
    }
}
//将指令设置界面新建的变量添加到xml文件中
void Widget::set_new_variable_from_instruction_set_widget_to_different_range_xml_file(QStandardItem *item, QString name){
    //首先判断作用域的类型
    QStandardItem* item_parent=item->parent();
    if(type_range[name]=="全局"){
        QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                TiXmlElement* node=new TiXmlElement("变量节点");
                //获取键值
                node->SetAttribute("Name",name.toStdString().c_str());
                node->SetAttribute("Type",type_attribute[name].toStdString().c_str());
                node->SetAttribute("Range",type_range[name].toStdString().c_str());
                root_robot->LinkEndChild(node);
                for(int i=0;i<item->rowCount();i++){
                    if(item->child(i,1)!=0){
                        //如果变量名的参数值不为0；将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item->child(i,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                }
            }
            doc.SaveFile();
        }
    }
    if(type_range[name]=="项目"){
        //获取项目路径
        QString project_name=ui->treeView_project->currentIndex().parent().data().toString();
        QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                TiXmlElement* node=new TiXmlElement("变量节点");//设置变量名
                //获取键值
                node->SetAttribute("Name",name.toStdString().c_str());
                QString type=ui->treeView_project->currentIndex().parent().data().toString();
                node->SetAttribute("Type",type_attribute[name].toStdString().c_str());
                node->SetAttribute("Range",type.toStdString().c_str());
                root_robot->LinkEndChild(node);
                for(int i=0;i<item->rowCount();i++){
                    if(item->child(i,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item->child(i,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                }
            }
            doc.SaveFile();
        }
    }
    if(type_range[name]=="程序"){
        //获取程序路径
        QString p2=ui->treeView_project->currentIndex().data().toString();
        //获取项目路径
        QString p1=ui->treeView_project->currentIndex().parent().data().toString();
        QString path=root_xml_file+"//"+p1+"//"+p2+".xml";
        TiXmlDocument doc;
        if(doc.LoadFile(path.toStdString().c_str())){
            TiXmlElement* root_robot=doc.RootElement();
            if(root_robot){
                TiXmlElement* node=new TiXmlElement("变量节点");//设置变量名
                //获取键
                node->SetAttribute("Name",name.toStdString().c_str());
                node->SetAttribute("Type",type_attribute[name].toStdString().c_str());
                QString temp1=ui->treeView_project->currentIndex().data().toString();
                QString temp2=ui->treeView_project->currentIndex().parent().data().toString();
                QString range=temp1+"["+temp2+"]";
                node->SetAttribute("Range",range.toStdString().c_str());
                root_robot->LinkEndChild(node);
                for(int i=0;i<item->rowCount();i++){
                    if(item->child(i,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item->child(i,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item->child(i,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node->LinkEndChild(value);
                    }
                }
            }
            doc.SaveFile();
        }
    }
}
//设置pos_AXISPOS的指令插入
void Widget::set_pos_AXIPOS_parameter(QStandardItem *item_variable){
    //在新建变量的后面插入相关的参数item
    QStandardItem* item_a1=new QStandardItem("a1:REAL");
    QStandardItem* item_a2=new QStandardItem("a2:REAL");
    QStandardItem* item_a3=new QStandardItem("a3:REAL");
    QStandardItem* item_a4=new QStandardItem("a4:REAL");
    QStandardItem* item_a5=new QStandardItem("a5:REAL");
    QStandardItem* item_a6=new QStandardItem("a6:REAL");
    //插入新建变量之后
    item_variable->appendRow(item_a1);
    item_variable->appendRow(item_a2);
    item_variable->appendRow(item_a3);
    item_variable->appendRow(item_a4);
    item_variable->appendRow(item_a5);
    item_variable->appendRow(item_a6);
    QModelIndex new_index=item_variable->index();
    ui->treeView_instruction_set->expand(new_index);
}
//设置pos_CARTPOS的指令插入
void Widget::set_pos_CARTPOS_parameter(QStandardItem *item_variable){
    //在新建变量的后面插入相关的参数item
    QStandardItem* item_a1=new QStandardItem("x:REAL");
    QStandardItem* item_a2=new QStandardItem("y:REAL");
    QStandardItem* item_a3=new QStandardItem("z:REAL");
    QStandardItem* item_a4=new QStandardItem("a:REAL");
    QStandardItem* item_a5=new QStandardItem("b:REAL");
    QStandardItem* item_a6=new QStandardItem("c:REAL");
    QStandardItem* item_a7=new QStandardItem("mode:DINT");
    //插入新建变量之后
    item_variable->appendRow(item_a1);
    item_variable->appendRow(item_a2);
    item_variable->appendRow(item_a3);
    item_variable->appendRow(item_a4);
    item_variable->appendRow(item_a5);
    item_variable->appendRow(item_a6);
    item_variable->appendRow(item_a7);
    QModelIndex new_index=item_variable->index();
    ui->treeView_instruction_set->expand(new_index);
}
//设置dyn的指令插入
void Widget::set_dyn_parameter(QStandardItem *item_variable){
    //在新建的变量后面插入相关的参数item
    QStandardItem* item_velAxis=new QStandardItem("velAxis:PERCENT");
    QStandardItem* item_accAxis=new QStandardItem("accAxis:PERCENT");
    QStandardItem* item_decAxis=new QStandardItem("decAxis:PERCENT");
    QStandardItem* item_jerkAxis=new QStandardItem("jerk:PERCENT");
    QStandardItem* item_vel=new QStandardItem("velPath:REAL");
    QStandardItem* item_acc=new QStandardItem("accPath:REAL");
    QStandardItem* item_dec=new QStandardItem("decPath:REAL");
    QStandardItem* item_jerk=new QStandardItem("jerkPath:REAL");
    QStandardItem* item_velOri=new QStandardItem("velOri:REAL");
    QStandardItem* item_accOri=new QStandardItem("accOri:REAL");
    QStandardItem* item_decOri=new QStandardItem("decOri:REAL");
    QStandardItem* item_jerkOri=new QStandardItem("jerkOri:REAL");
    //插入新建变量之后
    item_variable->appendRow(item_velAxis);
    item_variable->appendRow(item_accAxis);
    item_variable->appendRow(item_decAxis);
    item_variable->appendRow(item_jerkAxis);
    item_variable->appendRow(item_vel);
    item_variable->appendRow(item_acc);
    item_variable->appendRow(item_dec);
    item_variable->appendRow(item_jerk);
    item_variable->appendRow(item_velOri);
    item_variable->appendRow(item_accOri);
    item_variable->appendRow(item_decOri);
    item_variable->appendRow(item_jerkOri);
    QModelIndex new_index=item_variable->index();
    ui->treeView_instruction_set->expand(new_index);

}

//程序管理模块：指令设置界面添加按钮的槽函数
void Widget::on_btn_instruction_add_clicked()
{
    QModelIndex index_instruction_choose=ui->treeView_instruction_choose->currentIndex();
    QStandardItem* item_instruction_choose=treemodel_instruction_choose->itemFromIndex(index_instruction_choose);
    QString instruction_choose_name=item_instruction_choose->text();
    //根据在treeview_instruction_choose上所选的不同指令，做出不同的相应
    if(instruction_choose_name=="PTP"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_PTP->itemFromIndex(index);
        QStandardItem* item_PTP=item->parent()->parent();//获取到PTP的item
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="pos:POSITION_"){
            //当父节点为pos参数，添加在根节点的第一个位置
            QString part1="PTP";
            QString part2="(";
            QString part3=")";
            QString PTP_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
            item_PTP->setText(PTP_name);
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //当父节点为dyn参数时，添加在根节点的第二个位置
            int len1=item_PTP->text().length();//获取当前指令的总长
            QString part1=item_PTP->text().left(4);//第一部分：PTP(
            QString temp=item_PTP->text().right(len1-4);
            int len2=temp.length();
            QString part2=temp.left(len2-1);//第二部分：pos的变量名
            QString part3=ui->treeView_instruction_set->currentIndex().data().toString();//第三部部分：dyn变量名
            QString PTP_name=part1+part2+","+part3+")";
            //判断item_PTP是否已经添加了dyn参数，若添加了，则不再settext
            QString judge=item_PTP->text();
            int count=0;
            for(int i=0;i<judge.length();i++){
                if(judge[i]==","){
                    count++;
                    break;
                }
            }
            if(count==0){
                item_PTP->setText(PTP_name);
            }
        }

    }
    if(instruction_choose_name=="Lin"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_Lin->itemFromIndex(index);
        QStandardItem* item_Lin=item->parent()->parent();//获取到PTP的item
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="pos:POSITION_"){
            //当父节点为pos参数，添加在根节点的第一个位置
            QString part1="Lin";
            QString part2="(";
            QString part3=")";
            QString Lin_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
            item_Lin->setText(Lin_name);
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //当父节点为dyn参数时，添加在根节点的第二个位置
            int len1=item_Lin->text().length();//获取当前指令的总长
            QString part1=item_Lin->text().left(4);//第一部分：PTP(
            QString temp=item_Lin->text().right(len1-4);
            int len2=temp.length();
            QString part2=temp.left(len2-1);//第二部分：pos的变量名
            QString part3=ui->treeView_instruction_set->currentIndex().data().toString();//第三部部分：dyn变量名
            QString Lin_name=part1+part2+","+part3+")";
            //判断item_PTP是否已经添加了dyn参数，若添加了，则不再settext
            QString judge=item_Lin->text();
            int count=0;
            for(int i=0;i<judge.length();i++){
                if(judge[i]==","){
                    count++;
                    break;
                }
            }
            if(count==0){
                item_Lin->setText(Lin_name);
            }
        }
    }
    if(instruction_choose_name=="Circ"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_Circ->itemFromIndex(index);
        QStandardItem* item_Circ=item->parent()->parent();//获取到PTP的item
        int count=0;
        for(int i=0;i<item_Circ->text().length();i++){
            if(item_Circ->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="circPos:POSITION_"){
            //判断当前插入的pos参数变量为第一个还是第二个
            if(count==0|count==2&&(item_Circ->text().indexOf("(",1)==-1)){
                //当父节点为pos参数，添加在根节点的第一个位置
                QString part1="Circ";
                QString part2="(";
                QString part3=")";
                QString Circ_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
                item_Circ->setText(Circ_name);
            }
            else{
                if(count!=1){
                    //插入的是第二个参数
                    QString circ_name=","+ui->treeView_instruction_set->currentIndex().data().toString();
                    int index=item_Circ->text().indexOf(")",1);
                    QString temp=item_Circ->text();
                    temp.insert(index,circ_name);
                    item_Circ->setText(temp);
                }
            }

        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn_DYNAMIC_(OPT)"){
            if(count!=2){
                //当父节点为dyn参数时，添加在根节点的第三个位置
                QString dyn_name=","+ui->treeView_instruction_set->currentIndex().data().toString();
                int index=item_Circ->text().indexOf(")",1);
                QString temp=item_Circ->text();
                temp.insert(index,dyn_name);
                item_Circ->setText(temp);
            }
        }
    }
    if(instruction_choose_name=="PTPRel"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_PTPrel->itemFromIndex(index);
        QStandardItem* item_PTPREL=item->parent()->parent();//获取到PTP的item
        int count=0;
        for(int i=0;i<item_PTPREL->text().length();i++){
            if(item_PTPREL->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dist:DISTANCE_"){
            //当父节点为pos参数，添加在根节点的第一个位置
            QString part1="PTPRel";
            QString part2="(";
            QString part3=")";
            QString PTPREL_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
            item_PTPREL->setText(PTPREL_name);
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            if(count==0){
                //当父节点为dyn参数时，添加在根节点的第二个位置
                QString PTPREL_name=","+ui->treeView_instruction_set->currentIndex().data().toString();
                int dyn_index=item_PTPREL->text().indexOf(")",1);
                QString insert=item_PTPREL->text().insert(dyn_index,PTPREL_name);
                item_PTPREL->setText(insert);
            }

        }
    }
    if(instruction_choose_name=="LinRel"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_Linrel->itemFromIndex(index);
        QStandardItem* item_linrel=item->parent()->parent();//获取到PTP的item
        int count=0;
        for(int i=0;i<item_linrel->text().length();i++){
            if(item_linrel->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dist:DISTANCE_"){
            //当父节点为pos参数，添加在根节点的第一个位置
            QString part1="LinRel";
            QString part2="(";
            QString part3=")";
            QString LINREL_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
            item_linrel->setText(LINREL_name);

        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            if(count==0){
                //当父节点为dyn参数时，添加在根节点的第二个位置
                QString PTPREL_name=","+ui->treeView_instruction_set->currentIndex().data().toString();
                int dyn_index=item_linrel->text().indexOf(")",1);
                QString insert=item_linrel->text().insert(dyn_index,PTPREL_name);
                item_linrel->setText(insert);
            }
        }
    }
    if(instruction_choose_name=="StopRobot"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_stoprobot->itemFromIndex(index);
        QStandardItem* item_stoprobot=item->parent()->parent();
        QString temp1="StopRobot";
        QString temp2="(";
        QString temp3=")";
        QString name=temp1+temp2+ui->treeView_instruction_set->currentIndex().data().toString()+temp3;
        item_stoprobot->setText(name);
    }
    if(instruction_choose_name=="RefRobotAxis"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_refrobotaxis->itemFromIndex(index);
        QStandardItem* item_refrobotaxis=item->parent()->parent();//获取到refrobotaxis的item
        int count=0;
        for(int i=0;i<item_refrobotaxis->text().length();i++){
            if(item_refrobotaxis->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="axis:ROBOTAXIS"){
            //添加的为所要回零的轴
            QString part1="RefRobotAxis";
            QString part2="(";
            QString part3=")";
            QString ref_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
            item_refrobotaxis->setText(ref_name);
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="addMoveTarget:REAL(OPT)"){
            //添加移动数值的参数
            if(count==0){
                //添加的第二个参数
                QString ref_name=","+item->child(0,1)->text();
                int add_index=item_refrobotaxis->text().indexOf(")",1);
                QString insert=item_refrobotaxis->text().insert(add_index,ref_name);
                item_refrobotaxis->setText(insert);
            }
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //添加辅助dyn参数
            if(count==1){
                //添加第三个参数
                QString dyn_name=","+ui->treeView_instruction_set->currentIndex().data().toString();
                int dyn_index=item_refrobotaxis->text().indexOf(")",1);
                QString insert=item_refrobotaxis->text().insert(dyn_index,dyn_name);
                item_refrobotaxis->setText(insert);
            }
        }
    }
    if(instruction_choose_name=="RefRobotAxisAsync"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_RefRobotAxisAsync->itemFromIndex(index);
        QStandardItem* item_RefRobotAxisAsync=item->parent()->parent();//获取到refrobotaxis的item
        QString part1="RefRobotAxisAsync";
        QString part2="(";
        QString part3=")";
        QString name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
        item_RefRobotAxisAsync->setText(name);
    }
    if(instruction_choose_name=="MoveRobotAxis"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemdoel_MoveRobotAxis->itemFromIndex(index);
        QStandardItem* item_MoveRobotAxis=item->parent()->parent();//获取到refrobotaxis的item
        int count=0;
        for(int i=0;i<item_MoveRobotAxis->text().length();i++){
            if(item_MoveRobotAxis->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="axis:ROBOTAXIS"){
            //添加的为所要回零的轴
            QString part1="MoveRobotAxis";
            QString part2="(";
            QString part3=")";
            QString ref_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
            item_MoveRobotAxis->setText(ref_name);
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="pos:REAL"){
            //添加目标数值的参数
            if(count==0){
                //添加的第二个参数
                QString ref_name=","+item->child(0,1)->text();
                int add_index=item_MoveRobotAxis->text().indexOf(")",1);
                QString insert=item_MoveRobotAxis->text().insert(add_index,ref_name);
                item_MoveRobotAxis->setText(insert);
            }
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //添加辅助dyn参数
            if(count==1){
                //添加第三个参数
                QString dyn_name=","+ui->treeView_instruction_set->currentIndex().data().toString();
                int dyn_index=item_MoveRobotAxis->text().indexOf(")",1);
                QString insert=item_MoveRobotAxis->text().insert(dyn_index,dyn_name);
                item_MoveRobotAxis->setText(insert);
            }
        }
    }
    if(instruction_choose_name=="Dyn"){
        //添加DYNAMIC参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_dyn->itemFromIndex(index);
        QStandardItem* item_dyn=item->parent()->parent();//获取到dyn的item
        QString part1="Dyn";
        QString part2="(";
        QString part3=")";
        QString dyn_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
        item_dyn->setText(dyn_name);
    }
    if(instruction_choose_name=="Dynovr"){
        //添加相应的整数或者浮点数参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemdoel_dynovr->itemFromIndex(index);
        QStandardItem* item_dynovr=item->parent();//获取到dynovr的item
        QString part1="DynOvr";
        QString part2="(";
        QString part3=")";
        QString dynovr_name=part1+part2+item->child(0,1)->text()+part3;
        item_dynovr->setText(dynovr_name);
    }
    if(instruction_choose_name=="Ramp"){
        //首先添加加速度类型参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_Ramp->itemFromIndex(index);
        QStandardItem* item_ramp=item->parent()->parent();//获取到ramp的item
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="type:RAMPTYPE"){
            //添加到根节点的第一个位置
            QString part1="Ramp";
            QString part2="(";
            QString part3=")";
            QString Ramp_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
            item_ramp->setText(Ramp_name);
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="param:REAL(OPT)"){
            //添加到根节点的第二个位置
            int len1=item_ramp->text().length();//当前当前指令的总长
            QString part1=item_ramp->text().left(5);//第一部分：Ramp(
            QString temp=item_ramp->text().right(len1-5);
            int len2=temp.length();
            QString part2=temp.left(len2-1);//第二部分：ramp的变量名
            QString part3=ui->treeView_instruction_set->currentIndex().child(0,1).data().toString();//第三部部分：ramp变量名
            QString ramp_name=part1+part2+","+part3+")";
            //判断item_PTP是否已经添加了dyn参数，若添加了，则不再settext
            QString judge=item_ramp->text();
            int count=0;
            for(int i=0;i<judge.length();i++){
                if(judge[i]==","){
                    count++;
                    break;
                }
            }
            if(count==0){
                item_ramp->setText(ramp_name);
            }
        }
    }
    if(instruction_choose_name=="Refsys"){
        //添加CARTREFSYS数据类型
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_Refsys->itemFromIndex(index);
        QStandardItem* item_refsys=item->parent()->parent();//获取到dynovr的item
        QString part1="RefSys";
        QString part2="(";
        QString part3=")";
        QString refsys_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
        item_refsys->setText(refsys_name);
    }
    if(instruction_choose_name=="Tool"){
        //添加相应的参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_tool->itemFromIndex(index);
        QStandardItem* item_tool=item->parent()->parent();//获取waittime的item
        QString part1="Tool";
        QString part2="(";
        QString part3=")";
        QString tool_name=part1+part2+ui->treeView_instruction_set->currentIndex().data().toString()+part3;
        item_tool->setText(tool_name);
    }
    if(instruction_choose_name=="WaitTime"){
        //添加相应的参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_waittime->itemFromIndex(index);
        QStandardItem* item_waittime=item->parent()->parent();
        QString part1="WaitTime";
        QString part2="(";
        QString part3=")";
        QString waittime_name=part1+part2+ui->treeView_instruction_set->currentIndex().child(0,1).data().toString()+part3;//该行做出修改，加child
        item_waittime->setText(waittime_name);
    }
    if(instruction_choose_name=="Stop"){
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_stop->itemFromIndex(index);
        QStandardItem* item_stop=item->parent()->parent();
        QString temp1="Stop";
        QString temp2="(";
        QString temp3=")";
        QString name=temp1+temp2+ui->treeView_instruction_set->currentIndex().data().toString()+temp3;
        item_stop->setText(name);
    }
    if(instruction_choose_name=="Notice"){
        //添加相应的参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_info->itemFromIndex(index);
        QStandardItem* item_info=item->parent()->parent();
        QString part1="Notice";
        QString part2="(";
        QString part3=")";
        QString info_name=part1+part2+item->child(0,1)->text()+part3;
        item_info->setText(info_name);
    }
    if(instruction_choose_name=="Warning"){
        //添加相应得参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_warning->itemFromIndex(index);
        QStandardItem* item_warning=item->parent()->parent();
        QString part1="Warning";
        QString part2="(";
        QString part3=")";
        QString warning_name=part1+part2+item->child(0,1)->text()+part3;
        item_warning->setText(warning_name);
    }
    if(instruction_choose_name=="Error"){
        //添加相应得参数
        QModelIndex index=ui->treeView_instruction_set->currentIndex();
        QStandardItem* item=treemodel_error->itemFromIndex(index);
        QStandardItem* item_error=item->parent()->parent();
        QString part1="Error";
        QString part2="(";
        QString part3=")";
        QString Error_name=part1+part2+item->child(0,1)->text()+part3;
        item_error->setText(Error_name);
    }
}
//程序管理模块：指令设置界面设置按钮的槽函数
void Widget::on_btn_instruction_set_clicked()
{   
    QModelIndex index_instruction_choose=ui->treeView_instruction_choose->currentIndex();
    QStandardItem* item_instruction_choose=treemodel_instruction_choose->itemFromIndex(index_instruction_choose);
    QString instruction_choose_name=item_instruction_choose->text();
    if(instruction_choose_name=="PTP"){
        instruction_set_value(treemodel_PTP);
    }
    if(instruction_choose_name=="Lin"){
        instruction_set_value(treemodel_Lin);
    }
    if(instruction_choose_name=="Circ"){
        instruction_set_value(treemodel_Circ);
    }
    if(instruction_choose_name=="PTPRel"){
        instruction_set_value(treemodel_PTPrel);
    }
    if(instruction_choose_name=="LinRel"){
        instruction_set_value(treemodel_Linrel);
    }
    if(instruction_choose_name=="RefRobotAxis"){
        instruction_set_value(treemodel_refrobotaxis);
    }
    if(instruction_choose_name=="RefRobotAxisAsync"){
        instruction_set_value(treemodel_RefRobotAxisAsync);
    }
    if(instruction_choose_name=="MoveRobotAxis"){
        instruction_set_value(treemdoel_MoveRobotAxis);
    }
    if(instruction_choose_name=="Dynovr"){
        instruction_set_value(treemdoel_dynovr);
    }
}
//程序管理模块：指令设置界面设置按钮的函数
void Widget::instruction_set_value(QStandardItemModel *b){
    QModelIndex index=ui->treeView_instruction_set->currentIndex();
    QStandardItem* current_item=b->itemFromIndex(index);//当前所选的item
    QStandardItem* parent_item=current_item->parent();//父亲item
    QString Variable_name=parent_item->text();//获取变量名称
    bool isOK;
    //创建一个临时对话框，输入程序名称并且获取用户输入的程序名称
    QString program_text=QInputDialog::getText(NULL,"Input Dialog","please input Variable_Value",QLineEdit::Normal,"Variable value",&isOK);
    //需要设计算法，求出当前索引的参数的行数
    int count_parameter;
    int count_variable;
    //计算当前程序的行数
    for(int i=0;i<parent_item->rowCount();i++){
        //若当前选中的参量==父亲item第i个参量
        if(index.data().toString()==parent_item->child(i,0)->text()){
            QStandardItem* item_value=new QStandardItem(program_text);
            parent_item->setChild(i,1,item_value);
        }
    }
}
//程序管理模块中指令设置界面的确定按钮：将指令插入文本中
void Widget::set_instruction_to_program_edit(){
    //获取当前插入的指令名称
    QString instruction_name=ui->treeView_instruction_set->currentIndex().data().toString();
    QModelIndex index_program=ui->treeView_project->currentIndex();
    QStandardItem* item_program=treemodel_project->itemFromIndex(index_program);//获取所加载程序的item
    QStandardItem* item_project=item_program->parent();//加载程序的item的父item
    QString program_name=item_program->text();
    QString project_name=item_project->text();
    QString file_name=root_prog_file+"//"+project_name+"//"+program_name+".prog";//当前所加载程序的路径名称


    //需要在所选中行的上一行添加指令
    if(line_program_edit==0){
        //如果文本中只有一行，需要作一定处理
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::StartOfLine);
        ui->program_textEdit->setTextCursor(cur);
        ui->program_textEdit->textCursor().insertText("\n");
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(line_program_edit-1);
        ui->program_textEdit->setTextCursor(QTextCursor(block));//将行数往上移了一行
        //开始插入指令
        ui->program_textEdit->textCursor().insertText(instruction_name);

    }
    else{
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(line_program_edit-1);
        ui->program_textEdit->setTextCursor(QTextCursor(block));//将行数往上移了一行
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::EndOfLine);
        ui->program_textEdit->setTextCursor(cur);
        ui->program_textEdit->textCursor().insertText("\n");
        ui->program_textEdit->textCursor().insertText(instruction_name);

    }
    //开始将指令写入文件中
    QFile file(file_name);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::information(this,"错误信息","当前程序无法保存");
    }
    else{
        //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
        //若当前选中文本，需要先将文本移动到文末
        int temp=line_program_edit;//保存文本当前的行数
        QTextCursor cur=ui->program_textEdit->textCursor();
        cur.movePosition(QTextCursor::End);
        cur.select(QTextCursor::BlockUnderCursor);
        cur.removeSelectedText();
        //删除完行数之后，将光标移动到刚指定的行上面
        QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
        ui->program_textEdit->setTextCursor(QTextCursor(block));

        QTextStream out(&file);//分行写入文件
        out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
        out<<"{"<<"\n";
        out<<ui->program_textEdit->toPlainText()<<"\n";
        out<<"}";

        //写完之后，在program_edit末尾添加一行<<END_OFF>>
        ui->program_textEdit->append("<<END_OFF>>");
    }

    //将设置的变量保存到对应的var文件中
//    set_parameter_to_var_file();
    //将所有变量保存到对应的xml文件中
//    set_parameter_to_xml_file();
}
//程序管理模块：确认按钮，插入变量信息到var文件中
void Widget::set_parameter_to_var_file(){
    //以PTP、Lin、Circ为划分
    QString instruction_choose_name=ui->treeView_instruction_choose->currentIndex().data().toString();//所选的指令：PTP、Lin、Circ、
    QString instruction_set_name=ui->treeView_instruction_set->currentIndex().data().toString();//所插入的指令名称，如PTP(ap1，d1)

    //这部分获取从指令中获取变量名称
    int len=instruction_set_name.length();//插入指令的长度
    int count=0;//索引
    for(int i=0;i<instruction_set_name.length();i++){
        if(instruction_set_name[i]=="("){
            break;
        }
        else{
            count++;
        }
    }
    QString temp1=instruction_set_name.right(len-count-1);//获取变量名，如ap1,d1)
    QString temp2=temp1.left(temp1.length()-1);//获取变量名，如ap1,d1或者直接是一个变量，那就是ap1
    QString instruction_x1=temp2.left(temp2.indexOf(",",1));//所插入的第一个变量
    int judge=temp2.indexOf(",",1);//判断插入的指令时一个还是两个变量,-1为只有一个变量
//    qDebug()<<"第一个变量="<<instruction_x1;
    if(instruction_choose_name=="PTP"){
        QString pos_text=get_var_parameter_format_value(instruction_x1,root_PTP,6,0);
//        qDebug()<<"所插入的pos_var变量为："<<pos_text;
        //将所得指令插入到var文件中
        save_parameter_to_var_file(pos_text);
        if(judge!=-1){
            //插入的指令变量有两个
            QString dyn_name=temp2.right(temp2.length()-temp2.indexOf(",")-1);//获取第二个变量名，即dyn的参数变量名
            QString dyn_text=get_var_parameter_format_value(dyn_name,root_PTP,12,1);
//            qDebug()<<"所插入的第二个变量名="<<dyn_name;
//            qDebug()<<"所出入的dyn_var变量为:"<<dyn_text;
            //将所得指令插入到var文件中
            save_parameter_to_var_file(dyn_text);
        }
    }
    if(instruction_choose_name=="Lin"){
        QString pos_text=get_var_parameter_format_value(instruction_x1,root_Lin,6,0);
        save_parameter_to_var_file(pos_text);
        if(judge!=-1){
            //插入的变量有两个
            QString dyn_name=temp2.right(temp2.length()-temp2.indexOf(",")-1);//获取第二个变量名，即dyn的参数变量名
            QString dyn_text=get_var_parameter_format_value(dyn_name,root_Lin,12,1);
            save_parameter_to_var_file(dyn_text);
        }
    }
    if(instruction_choose_name=="Circ"){
        QString pos_text=get_var_parameter_format_value(instruction_x1,root_Circ,6,0);
        save_parameter_to_var_file(pos_text);
        if(judge!=-1){
            //插入的变量有两个
            QString dyn_name=temp2.right(temp2.length()-temp2.indexOf(",")-1);//获取第二个变量名，即dyn的参数变量名
            QString dyn_text=get_var_parameter_format_value(dyn_name,root_Circ,12,1);
            save_parameter_to_var_file(dyn_text);
        }
    }
}
//通过该函数来获取所要添加的var文件的变量格式的名称
QString Widget::get_var_parameter_format_value(QString parameter_name,QStandardItem* root,int parameter_number,int parameter_type){
    vector<QString> str(parameter_number+1);

    //判断第一个指令在pos下的位置，读取变量
    QStandardItem* item=root->child(0,0)->child(parameter_type,0);//获取pos的item
    QStandardItem* current_item;
    for(int i=0;i<item->rowCount();i++){
        if(parameter_name==item->child(i,0)->text()){
            current_item=item->child(i,0);//item_pos为当前添加的pos item
        }
    }
    QString attribute_name=current_item->text();//获取变量名对应数据类型的键值
    //获取pos变量名下的若干个个参数的数值
    if(type_attribute[attribute_name]=="AXISPOS"|type_attribute[attribute_name]=="DYNAMIC"){
        for(int i=0;i<parameter_number;i++){
            if(current_item->child(i,1)!=0){
                str[i]=current_item->child(i,1)->text();
            }
            else{
                str[i]="0";
            }
        }
    }
    if(type_attribute[attribute_name]=="CARTPOS"){
        for(int i=0;i<parameter_number+1;i++){
            if(current_item->child(i,1)!=0){
                str[i]=current_item->child(i,1)->text();
            }
            else{
                str[i]="0";
            }
        }
    }
    //开始创建变量参数存储格式：变量名：变量类型=数据类型
    //根据不同的parameter_type进行不同的设置
    if(parameter_type==0){
        //此时有效对象为：pos参数变量名
        if(type_attribute[attribute_name]=="AXISPOS"){
            QString value=current_item->text()+":"+type_attribute[attribute_name]+"=(a1="+str[0]+",a2="+str[1]+",a3="+str[2]+",a4="+str[3]+",a5="+str[4]+",a6="+str[5]+")";
            return  value;
        }
        if(type_attribute[attribute_name]=="CARTPOS"){
            QString value=current_item->text()+":"+type_attribute[attribute_name]+"=(x="+str[0]+",y="+str[1]+",z="+str[2]+",a="+str[3]+",b="+str[4]+",c="+str[5]+",mode="+str[6]+")";
            return  value;
        }
    }
    if(parameter_type==1){
        //此时有效对象为：dyn参数变量名
        QString value=current_item->text()+":"+type_attribute[attribute_name]+"=("+"velAxis="+str[0]+",accAxis="+str[1]+",decAxis="+str[2]+",jerkAxis="+str[3]+",vel="+str[4]+",acc="+str[5]+",dec="+str[6]+",jerk="+str[7]+",velOri="+str[8]+",accOri="+str[9]+",decOri="+str[10]+",jerkOri="+str[11]+")";
        return value;
    }
}
//将所得指令插入到var文件中的函数
void Widget::save_parameter_to_var_file(QString parameter_name){
    //首先获取当前的所加载程序对应的var路径
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* current_item=treemodel_project->itemFromIndex(index);//当前所加载的item
    QStandardItem* parent=current_item->parent();//父亲item
    QString program_name=current_item->text();
    QString project_name=parent->text();
    QString path=root_var_file+"//"+project_name+"//"+program_name+".var";//文件保存的路径
    QFile file(path);
    file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append);
    //如果var文件为空
    if(file.size()==0){
        file.write("{");
        file.write("\n");
        file.write(parameter_name.toStdString().c_str());
        file.write("\n");
        file.write("}");
        file.close();
    }
    //如果var文件不为空
    else{
        vector<string> content;
        string strline;
        ifstream infile(path.toStdString());
        while(infile){
            getline(infile,strline);
            content.push_back(strline);
        }
        infile.close();
        content.erase(content.end());

        ofstream outfile(path.toStdString());
        vector<string>::const_iterator iter=content.begin();
        for(;content.end()-1!=iter;++iter){
            outfile.write((*iter).c_str(),(*iter).size());
            outfile<<'\n';
        }
        outfile.close();

        QFile file_2(path.toStdString().c_str());
        file_2.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append);
        file_2.write(parameter_name.toStdString().c_str());
        file_2.write("\n");
        file_2.write("}");
        file_2.close();
    }
}
//程序管理模块：确认按钮，插入变量信息到xml文件中
void Widget::set_parameter_to_xml_file(){
    //获取当前treeview_instruction_choose中所选的指令名称
    QModelIndex index=ui->treeView_instruction_choose->currentIndex();
    QStandardItem* item=treemodel_instruction_choose->itemFromIndex(index);
    QString instruction_choose_name=item->text();//指令选择窗口所选你的程序名称
    if(instruction_choose_name=="PTP"){
        set_different_instruction_parameter_to_xml_file(root_PTP);//将treeivew_instruction_set的变量信息插入到对应的xml文件中(只包含程序作用域的变量)
        set_different_instruction_parameter_to_project_range_xml_file(root_PTP);//该函数生成项目作用域的xml文件
        set_different_instruction_parameter_to_all_range_xml_file(root_PTP);//该函数生成全局作用域的xml文件
    }
    if(instruction_choose_name=="Lin"){
        set_different_instruction_parameter_to_xml_file(root_Lin);
        set_different_instruction_parameter_to_project_range_xml_file(root_Lin);
        set_different_instruction_parameter_to_all_range_xml_file(root_Lin);
    }
    if(instruction_choose_name=="Circ"){
        set_different_instruction_parameter_to_xml_file(root_Circ);
        set_different_instruction_parameter_to_project_range_xml_file(root_Circ);
        set_different_instruction_parameter_to_all_range_xml_file(root_Circ);
    }
}
//将treeview_instruction_set上全局作用域的变量存储到该文件
void Widget::set_different_instruction_parameter_to_all_range_xml_file(QStandardItem *root){
    //创建一个xml文档
    TiXmlDocument doc;
    //添加文档头部的基本声明
    TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","UTF-8","yes");
    doc.LinkEndChild(dec);
    //为文档添加基本的注释
    TiXmlComment* comment=new TiXmlComment("This is Robot_Variable Information");
    doc.LinkEndChild(comment);
    //为xml文档添加根元素
    TiXmlElement* root_robot=new TiXmlElement("config_information");

    //开始写数据进xml文件中
    QStandardItem* item_pos=root->child(0,0)->child(0,0);//首先获取pos_item
    if(item_pos->rowCount()!=0){
        //不为零，证明pos参数下有变量
        vector<TiXmlElement*> node_node_pos(item_pos->rowCount());
        for(int i=0;i<item_pos->rowCount();i++){
            //需要判断变量的作用域，只有全局作用域，才生成
            QModelIndex index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(index);
            QStandardItem* parent_item=current_item->parent();
            QString program_name=current_item->text();
            QString project_name=parent_item->text();//当前加载的程序名称
            if(type_range[item_pos->child(i,0)->text()]!=project_name&&type_range[item_pos->child(i,0)->text()]!=program_name){
                node_node_pos[i]=new TiXmlElement("pos");
                node_node_pos[i]->SetAttribute("Name",item_pos->child(i,0)->text().toStdString().c_str());//添加变量节点的变量名
                node_node_pos[i]->SetAttribute("Type",type_attribute[item_pos->child(i,0)->text()].toStdString().c_str());//添加数据类型
                node_node_pos[i]->SetAttribute("Range",type_range[item_pos->child(i,0)->text()].toStdString().c_str());//添加作用域
                root_robot->LinkEndChild(node_node_pos[i]);//根节点连接子节点
                for(int j=0;j<item_pos->child(i,0)->rowCount();j++){
                    if(item_pos->child(i,0)->child(j,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parmeter",item_pos->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item_pos->child(i,0)->child(j,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node_node_pos[i]->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item_pos->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node_node_pos[i]->LinkEndChild(value);
                    }
                }

            }

        }
    }
    QStandardItem* item_dyn=root->child(0,0)->child(1,0);//获取dyn_item
    if(item_dyn->rowCount()!=0){
        //不为零，证明dyn参数下有变量
        vector<TiXmlElement*> node_node_dyn(item_dyn->rowCount());
        for(int i=0;i<item_dyn->rowCount();i++){
            //需要判断变量的作用域，只有程序作用域，才生成
            QModelIndex index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(index);
            QStandardItem* parent_item=current_item->parent();
            QString program_name=current_item->text();
            QString project_name=parent_item->text();//当前加载的程序名称
            if(type_range[item_dyn->child(i,0)->text()]!=project_name&&type_range[item_dyn->child(i,0)->text()]!=program_name){
                node_node_dyn[i]=new TiXmlElement("dyn");
                node_node_dyn[i]->SetAttribute("Name",item_dyn->child(i,0)->text().toStdString().c_str());//添加变量节点的属性
                node_node_dyn[i]->SetAttribute("Type",type_attribute[item_dyn->child(i,0)->text()].toStdString().c_str());//添加数据类型
                node_node_dyn[i]->SetAttribute("Range",type_range[item_dyn->child(i,0)->text()].toStdString().c_str());//添加作用域
                root_robot->LinkEndChild(node_node_dyn[i]);//根节点连接子节点
                for(int j=0;j<item_dyn->child(i,0)->rowCount();j++){
                    if(item_dyn->child(i,0)->child(j,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item_dyn->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item_dyn->child(i,0)->child(j,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node_node_dyn[i]->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动复制为0，写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item_dyn->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node_node_dyn[i]->LinkEndChild(value);
                    }
                }
            }

        }

    }
    doc.LinkEndChild(root_robot);//插入根节点
    //获取程序保存路径
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* item=treemodel_project->itemFromIndex(index);
    QStandardItem* parent_item=item->parent();
    QString project_name=parent_item->text();
    QString path=root_xml_file+"//"+"all.xml";//文件保存的路径
    doc.SaveFile(path.toStdString().c_str());//将xml文件写入对应的文件路径
    doc.Print();
}
//将treeview_instruction_set上项目作用域的变量存储到该文件
void Widget::set_different_instruction_parameter_to_project_range_xml_file(QStandardItem *root){
    //创建一个xml文档
    TiXmlDocument doc;
    //添加文档头部的基本声明
    TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","UTF-8","yes");
    doc.LinkEndChild(dec);
    //为文档添加基本的注释
    TiXmlComment* comment=new TiXmlComment("This is Robot_Variable Information");
    doc.LinkEndChild(comment);
    //为xml文档添加根元素
    TiXmlElement* root_robot=new TiXmlElement("config_information");

    //开始写数据进xml文件中
    QStandardItem* item_pos=root->child(0,0)->child(0,0);//首先获取pos_item
    if(item_pos->rowCount()!=0){
        //不为零，证明pos参数下有变量
        vector<TiXmlElement*> node_node_pos(item_pos->rowCount());
        for(int i=0;i<item_pos->rowCount();i++){
            //需要判断变量的作用域，只有程序作用域，才生成
            QModelIndex index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(index);
            QStandardItem* parent_item=current_item->parent();
            QString project_name=parent_item->text();//当前加载的程序名称
            if(type_range[item_pos->child(i,0)->text()]==project_name){
                node_node_pos[i]=new TiXmlElement("pos");
                node_node_pos[i]->SetAttribute("Name",item_pos->child(i,0)->text().toStdString().c_str());//添加变量节点的变量名
                node_node_pos[i]->SetAttribute("Type",type_attribute[item_pos->child(i,0)->text()].toStdString().c_str());//添加数据类型
                node_node_pos[i]->SetAttribute("Range",type_range[item_pos->child(i,0)->text()].toStdString().c_str());//添加作用域
                root_robot->LinkEndChild(node_node_pos[i]);//根节点连接子节点
                for(int j=0;j<item_pos->child(i,0)->rowCount();j++){
                    if(item_pos->child(i,0)->child(j,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parmeter",item_pos->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item_pos->child(i,0)->child(j,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node_node_pos[i]->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item_pos->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node_node_pos[i]->LinkEndChild(value);
                    }
                }

            }

        }
    }
    QStandardItem* item_dyn=root->child(0,0)->child(1,0);//获取dyn_item
    if(item_dyn->rowCount()!=0){
        //不为零，证明dyn参数下有变量
        vector<TiXmlElement*> node_node_dyn(item_dyn->rowCount());
        for(int i=0;i<item_dyn->rowCount();i++){
            //需要判断变量的作用域，只有程序作用域，才生成
            QModelIndex index=ui->treeView_project->currentIndex();
            QStandardItem* current_item=treemodel_project->itemFromIndex(index);
            QStandardItem* parent_item=current_item->parent();
            QString project_name=parent_item->text();//当前加载的程序名称
            if(type_range[item_dyn->child(i,0)->text()]==project_name){
                node_node_dyn[i]=new TiXmlElement("dyn");
                node_node_dyn[i]->SetAttribute("Name",item_dyn->child(i,0)->text().toStdString().c_str());//添加变量节点的属性
                node_node_dyn[i]->SetAttribute("Type",type_attribute[item_dyn->child(i,0)->text()].toStdString().c_str());//添加数据类型
                node_node_dyn[i]->SetAttribute("Range",type_range[item_dyn->child(i,0)->text()].toStdString().c_str());//添加作用域
                root_robot->LinkEndChild(node_node_dyn[i]);//根节点连接子节点
                for(int j=0;j<item_dyn->child(i,0)->rowCount();j++){
                    if(item_dyn->child(i,0)->child(j,1)!=0){
                        //如果变量名下的参数值不为0，将当前值写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item_dyn->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText(item_dyn->child(i,0)->child(j,1)->text().toStdString().c_str());
                        value->LinkEndChild(value_text);
                        node_node_dyn[i]->LinkEndChild(value);
                    }
                    else{
                        //若当前变量名下的参数值为0，将自动复制为0，写入节点中
                        TiXmlElement* value=new TiXmlElement("value");
                        value->SetAttribute("Parameter",item_dyn->child(i,0)->child(j,0)->text().toStdString().c_str());
                        TiXmlText* value_text=new TiXmlText("0");
                        value->LinkEndChild(value_text);
                        node_node_dyn[i]->LinkEndChild(value);
                    }
                }
            }

        }

    }
    doc.LinkEndChild(root_robot);//插入根节点
    //获取程序保存路径
    QModelIndex index=ui->treeView_project->currentIndex();
    QStandardItem* item=treemodel_project->itemFromIndex(index);
    QStandardItem* parent_item=item->parent();
    QString project_name=parent_item->text();
    QString path=root_xml_file+"//"+project_name+"//"+project_name+".xml";//文件保存的路径
    doc.SaveFile(path.toStdString().c_str());//将xml文件写入对应的文件路径
}
//将treeivew_instruction_set当前的变量信息存储到对应的xml文件中
void Widget::set_different_instruction_parameter_to_xml_file(QStandardItem *root){

//    //创建一个xml文档
//    TiXmlDocument doc;
//    //添加文档头部的基本声明
//    TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","UTF-8","yes");
//    doc.LinkEndChild(dec);
//    //为文档添加基本的注释
//    TiXmlComment* comment=new TiXmlComment("This is Robot_Variable Information");
//    doc.LinkEndChild(comment);
//    //为xml文档添加根元素
//    TiXmlElement* root_robot=new TiXmlElement("config_information");

//    //开始写数据进xml文件中
//    QStandardItem* item_pos=root->child(0,0)->child(0,0);//首先获取pos_item
//    if(item_pos->rowCount()!=0){
//        //不为零，证明pos参数下有变量
//        TiXmlElement* node_node_pos[item_pos->rowCount()];
//        for(int i=0;i<item_pos->rowCount();i++){
//            //需要判断变量的作用域，只有程序作用域，才生成
//            QModelIndex index=ui->treeView_project->currentIndex();
//            QStandardItem* current_item=treemodel_project->itemFromIndex(index);
//            QString program_name=current_item->text();//当前加载的程序名称
//            if(type_range[item_pos->child(i,0)->text()]==program_name){
//                node_node_pos[i]=new TiXmlElement("pos");
//                node_node_pos[i]->SetAttribute("Name",item_pos->child(i,0)->text().toStdString().c_str());//添加变量节点的变量名
//                node_node_pos[i]->SetAttribute("Type",type_attribute[item_pos->child(i,0)->text()].toStdString().c_str());//添加数据类型
//                node_node_pos[i]->SetAttribute("Range",type_range[item_pos->child(i,0)->text()].toStdString().c_str());//添加作用域
//                root_robot->LinkEndChild(node_node_pos[i]);//根节点连接子节点
//                for(int j=0;j<item_pos->child(i,0)->rowCount();j++){
//                    if(item_pos->child(i,0)->child(j,1)!=0){
//                        //如果变量名下的参数值不为0，将当前值写入节点中
//                        TiXmlElement* value=new TiXmlElement("value");
//                        value->SetAttribute("Parmeter",item_pos->child(i,0)->child(j,0)->text().toStdString().c_str());
//                        TiXmlText* value_text=new TiXmlText(item_pos->child(i,0)->child(j,1)->text().toStdString().c_str());
//                        value->LinkEndChild(value_text);
//                        node_node_pos[i]->LinkEndChild(value);
//                    }
//                    else{
//                        //若当前变量名下的参数值为0，将自动赋值为0，写入节点当中
//                        TiXmlElement* value=new TiXmlElement("value");
//                        value->SetAttribute("Parameter",item_pos->child(i,0)->child(j,0)->text().toStdString().c_str());
//                        TiXmlText* value_text=new TiXmlText("0");
//                        value->LinkEndChild(value_text);
//                        node_node_pos[i]->LinkEndChild(value);
//                    }
//                }

//            }

//        }
//    }
//    QStandardItem* item_dyn=root->child(0,0)->child(1,0);//获取dyn_item
//    if(item_dyn->rowCount()!=0){
//        //不为零，证明dyn参数下有变量
//        TiXmlElement* node_node_dyn[item_dyn->rowCount()];
//        for(int i=0;i<item_dyn->rowCount();i++){
//            //需要判断变量的作用域，只有程序作用域，才生成
//            QModelIndex index=ui->treeView_project->currentIndex();
//            QStandardItem* current_item=treemodel_project->itemFromIndex(index);
//            QString program_name=current_item->text();//当前加载的程序名称
//            if(type_range[item_dyn->child(i,0)->text()]==program_name){
//                node_node_dyn[i]=new TiXmlElement("dyn");
//                node_node_dyn[i]->SetAttribute("Name",item_dyn->child(i,0)->text().toStdString().c_str());//添加变量节点的属性
//                node_node_dyn[i]->SetAttribute("Type",type_attribute[item_dyn->child(i,0)->text()].toStdString().c_str());//添加数据类型
//                node_node_dyn[i]->SetAttribute("Range",type_range[item_dyn->child(i,0)->text()].toStdString().c_str());//添加作用域
//                root_robot->LinkEndChild(node_node_dyn[i]);//根节点连接子节点
//                for(int j=0;j<item_dyn->child(i,0)->rowCount();j++){
//                    if(item_dyn->child(i,0)->child(j,1)!=0){
//                        //如果变量名下的参数值不为0，将当前值写入节点中
//                        TiXmlElement* value=new TiXmlElement("value");
//                        value->SetAttribute("Parameter",item_dyn->child(i,0)->child(j,0)->text().toStdString().c_str());
//                        TiXmlText* value_text=new TiXmlText(item_dyn->child(i,0)->child(j,1)->text().toStdString().c_str());
//                        value->LinkEndChild(value_text);
//                        node_node_dyn[i]->LinkEndChild(value);
//                    }
//                    else{
//                        //若当前变量名下的参数值为0，将自动复制为0，写入节点中
//                        TiXmlElement* value=new TiXmlElement("value");
//                        value->SetAttribute("Parameter",item_dyn->child(i,0)->child(j,0)->text().toStdString().c_str());
//                        TiXmlText* value_text=new TiXmlText("0");
//                        value->LinkEndChild(value_text);
//                        node_node_dyn[i]->LinkEndChild(value);
//                    }
//                }
//            }

//        }

//    }
//    doc.LinkEndChild(root_robot);//插入根节点
//    //获取程序保存路径
//    QModelIndex index=ui->treeView_project->currentIndex();
//    QStandardItem* item=treemodel_project->itemFromIndex(index);
//    QStandardItem* parent_item=item->parent();
//    QString program_name=item->text();
//    QString project_name=parent_item->text();
//    QString path=root_xml_file+"//"+project_name+"//"+program_name+".xml";//文件保存的路径
//    doc.SaveFile(path.toStdString().c_str());//将xml文件写入对应的文件路径
}
//布局位置管理模块的基本界面
void Widget::set_coordinate_charge_widget(){
    //首先为点动速度按钮添加菜单选项
    QMenu* coordinate_inching_speed_menu=new QMenu(ui->page_coordinate);
    ui->btn_coordinate_inching_speed->setMenu(coordinate_inching_speed_menu);
    //初始化点动速度下的若干选项
    inching_speed_01=new QAction("0.1 lnc",ui->page_coordinate);
    inching_speed_1=new QAction("1.0 lnc",ui->page_coordinate);
    inching_speed_10=new QAction("10%",ui->page_coordinate);
    inching_speed_25=new QAction("25%",ui->page_coordinate);
    inching_speed_50=new QAction("50%",ui->page_coordinate);
    inching_speed_100=new QAction("100%",ui->page_coordinate);
    //将控件添加进menu中
    coordinate_inching_speed_menu->addAction(inching_speed_01);
    coordinate_inching_speed_menu->addAction(inching_speed_1);
    coordinate_inching_speed_menu->addAction(inching_speed_10);
    coordinate_inching_speed_menu->addAction(inching_speed_25);
    coordinate_inching_speed_menu->addAction(inching_speed_50);
    coordinate_inching_speed_menu->addAction(inching_speed_100);
    //为电动按钮添加选项
    QMenu* coordinate_inching_menu=new QMenu(ui->page_coordinate);
    ui->btn_coordinate_inching->setMenu(coordinate_inching_menu);
    //初始化点动按钮下的若干个按钮
    inching_tool=new QAction("工具坐标",ui->page_coordinate);
    inching_world=new QAction("世界坐标",ui->page_coordinate);
    inching_axis=new QAction("轴坐标",ui->page_coordinate);
    //将控件添加进menu中
    coordinate_inching_menu->addAction(inching_tool);
    coordinate_inching_menu->addAction(inching_world);
    coordinate_inching_menu->addAction(inching_axis);

    //添加相应的信号槽
    //点动速度按钮的信号槽
    connect(inching_speed_01,SIGNAL(triggered()),this,SLOT(slt_inching_speed_01()));
    connect(inching_speed_1,SIGNAL(triggered()),this,SLOT(slt_inching_speed_1()));
    connect(inching_speed_10,SIGNAL(triggered()),this,SLOT(slt_icching_speed_10()));
    connect(inching_speed_25,SIGNAL(triggered()),this,SLOT(slt_inching_speed_25()));
    connect(inching_speed_50,SIGNAL(triggered()),this,SLOT(slt_inching_speed_50()));
    connect(inching_speed_100,SIGNAL(triggered()),this,SLOT(slt_inching_speed_100()));
    //点动按钮的信号槽
    connect(inching_tool,SIGNAL(triggered()),this,SLOT(slt_inching_tool()));
    connect(inching_world,SIGNAL(triggered()),this,SLOT(slt_inching_world()));
    connect(inching_axis,SIGNAL(triggered()),this,SLOT(slt_inching_axis()));

    //开始布局位置管理模块显示坐标的视图
    tablemodel_coordinate=new QStandardItemModel(6,3,ui->page_coordinate);
    tablemodel_coordinate->setColumnCount(3);
    tablemodel_coordinate->setHeaderData(0,Qt::Horizontal,"名称");
    tablemodel_coordinate->setHeaderData(1,Qt::Horizontal,"数值");
    tablemodel_coordinate->setHeaderData(2,Qt::Horizontal,"单位");
    ui->tableView_coordinate->setModel(tablemodel_coordinate);
    ui->tableView_coordinate->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_coordinate->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_coordinate->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_coordinate->verticalHeader()->setStretchLastSection(true);
}
//位置管理模块：伺服按钮的槽函数
void Widget::on_btn_coordinate_servo_clicked()
{
    //开始写数据进模型中
    QString str[6]={"电机轴1","电机轴2","电机轴3","电机轴4","电机轴5","电机轴6"};
    for(int i=0;i<6;i++){
        QStandardItem* item=new QStandardItem(str[i]);
        item->setTextAlignment(Qt::AlignCenter);
        tablemodel_coordinate->setItem(i,0,item);
        QStandardItem* item_unit=new QStandardItem("度");
        item_unit->setTextAlignment(Qt::AlignCenter);
        tablemodel_coordinate->setItem(i,2,item_unit);
    }
    coordinateShow = 1;
}

//位置管理模块：关节按钮的槽函数
void Widget::on_btn_coordinate_joint_clicked()
{
    //开始写数据进模型中
    QString str_coordinate[6]={"A1","A2","A3","A4","A5","A6"};
    for(int i=0;i<6;i++){
        QStandardItem* item=new QStandardItem(str_coordinate[i]);
        item->setTextAlignment(Qt::AlignCenter);
        tablemodel_coordinate->setItem(i,0,item);
        QStandardItem* item_unit=new QStandardItem("度");
        item_unit->setTextAlignment(Qt::AlignCenter);
        tablemodel_coordinate->setItem(i,2,item_unit);
    }
    coordinateShow = 2;

}
//位置管理模块：世界按钮的槽函数
void Widget::on_btn_coordinate_world_clicked()
{
    //开始写数据进模型中
    QString str_coordinate[6]={"X","Y","Z","A","B","C"};
    QString str[6]={"毫米","毫米","毫米","度","度","度"};
    for(int i=0;i<6;i++){
        QStandardItem* item=new QStandardItem(str_coordinate[i]);
        item->setTextAlignment(Qt::AlignCenter);
        tablemodel_coordinate->setItem(i,0,item);
        QStandardItem* item_unit=new QStandardItem(str[i]);
        item_unit->setTextAlignment(Qt::AlignCenter);
        tablemodel_coordinate->setItem(i,2,item_unit);
    }
    coordinateShow = 3;
}
//位置管理模块：电动速度的槽函数
void Widget::slt_inching_speed_01(){
    ui->label_coordinate_inching_speed_2->setText("0.1 lnc");
}
void Widget::slt_inching_speed_1(){
    ui->label_coordinate_inching_speed_2->setText("1.0 lnc");
}
void Widget::slt_icching_speed_10(){
    ui->label_coordinate_inching_speed_2->setText("10%");
}
void Widget::slt_inching_speed_25(){
    ui->label_coordinate_inching_speed_2->setText("25%");
}
void Widget::slt_inching_speed_50(){
    ui->label_coordinate_inching_speed_2->setText("50%");
}
void Widget::slt_inching_speed_100(){
    ui->label_coordinate_inching_speed_2->setText("100%");
}
//位置管理模块：点动按钮的槽函数
void Widget::slt_inching_axis(){
    qDebug()<<"切换到axis坐标系";
}
void Widget::slt_inching_tool(){
    qDebug()<<"切换到tool坐标系";
}
void Widget::slt_inching_world(){
    qDebug()<<"切换到world坐标系";
}
//布局位置管理模块的函数
void Widget::set_information_charge_widget(){
    //开始布局信息报告管理模块显示部分的视图
    tablemodel_information=new QStandardItemModel(15,4,ui->page_information);
    tablemodel_information->setColumnCount(4);
    tablemodel_information->setHeaderData(0,Qt::Horizontal,"类别");
    tablemodel_information->setHeaderData(1,Qt::Horizontal,"时间");
    tablemodel_information->setHeaderData(2,Qt::Horizontal,"状态");
    tablemodel_information->setHeaderData(3,Qt::Horizontal,"描述");
    ui->tableView_information->setModel(tablemodel_information);
    ui->tableView_information->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_information->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_information->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_information->verticalHeader()->setStretchLastSection(true);
}
//程序管理模块：指令修改界面的槽函数
//返回按钮，返回到程序管理模块的编辑界面
void Widget::on_btn_program_edit_back_clicked()
{
    //首先点击按钮，将返回到程序管理模块的编辑界面
    int indexpage=0;
    switch_page_program(indexpage);
}

//添加按钮：将变量添加到对应得指令界面中
void Widget::on_btn_program_edit_add_clicked()
{
    //获取光标所在行得内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    QString instruction=cur.selectedText();
    QString judge=instruction.left(instruction.indexOf("(",1));
    //根据不同得指令，作出不同得相应
    if(judge=="PTP"){
        //获取当前光标所获取得item
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_PTP->itemFromIndex(index);
        QStandardItem* item_PTP=item->parent()->parent();//获取PTP得item
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="pos:POSITION_"){
            //当父节点为pos参数，添加再根节点得第一个位置
            QString part1="PTP";
            QString part2="(";
            QString part3=")";
            QString PTP_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
            item_PTP->setText(PTP_name);
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //当父节点为dyn参数时，添加在根节点的第二个位置
            int len1=item_PTP->text().length();//获取当前指令得总长
            QString part1=item_PTP->text().left(4);//第一部分：PTP(
            QString temp=item_PTP->text().right(len1-4);
            int len2=temp.length();
            QString part2=temp.left(len2-1);//第二部分：pos的变量名
            QString part3=ui->treeView_program_correct->currentIndex().data().toString();
            QString PTP_name=part1+part2+","+part3+")";
            //判断item_PTP是否已经添加了dyn参数，若添加了，则不再settext
            QString judge=item_PTP->text();
            int count=0;
            for(int i=0;i<judge.length();i++){
                if(judge[i]==","){
                    count++;
                    break;
                }
            }
            if(count==0){
                item_PTP->setText(PTP_name);
            }
        }
    }
    if(judge=="Lin"){
        //获取当前光标所获取得item
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_Lin->itemFromIndex(index);
        QStandardItem* item_Lin=item->parent()->parent();//获取Lin得item
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="pos:POSITION_"){
            //当父节点为pos参数，添加再根节点得第一个位置
            QString part1="Lin";
            QString part2="(";
            QString part3=")";
            QString Lin_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
            item_Lin->setText(Lin_name);
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //当父节点为dyn参数时，添加在根节点的第二个位置
            int len1=item_Lin->text().length();//获取当前指令的总长
            QString part1=item_Lin->text().left(4);//第一部分：PTP(
            QString temp=item_Lin->text().right(len1-4);
            int len2=temp.length();
            QString part2=temp.left(len2-1);//第二部分：pos的变量名
            QString part3=ui->treeView_program_correct->currentIndex().data().toString();//第三部分：dyn变量名
            QString Lin_name=part1+part2+","+part3+")";
            //判断item_PTP是否已经添加了dyn参数，若添加了，则不再settext
            QString judge=item_Lin->text();
            int count=0;
            for(int i=0;i<judge.length();i++){
                if(judge[i]==","){
                    count++;
                    break;
                }
            }
            if(count==0){
                item_Lin->setText(Lin_name);
            }
        }
    }
    if(judge=="Circ"){
        //获取当前光标所获取得item
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_Circ->itemFromIndex(index);
        QStandardItem* item_Circ=item->parent()->parent();//获取Lin得item
        int count=0;
        for(int i=0;i<item_Circ->text().length();i++){
            if(item_Circ->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="circPos:POSITION_"){
            //判断当前插入的pos参数变量为第一个还是第二个
            if(count==0|count==2&&(item_Circ->text().indexOf("(",1)==-1)){
                //当父节点为pos参数，添加在根节点的第一个位置
                QString part1="Circ";
                QString part2="(";
                QString part3=")";
                QString Circ_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
                item_Circ->setText(Circ_name);
            }
            else{
                if(count!=1){
                    //插入的是第二个参数
                    QString circ_name=","+ui->treeView_program_correct->currentIndex().data().toString();
                    int index=item_Circ->text().indexOf(")",1);
                    QString temp=item_Circ->text();
                    temp.insert(index,circ_name);
                    item_Circ->setText(temp);
                }
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dyn_DYNAMIC_(OPT)"){
            if(count!=2){
                //当父节点为dyn参数时，添加在根节点的第三个位置
                QString dyn_name=","+ui->treeView_program_correct->currentIndex().data().toString();
                int index=item_Circ->text().indexOf(")",1);
                QString temp=item_Circ->text();
                temp.insert(index,dyn_name);
                item_Circ->setText(temp);
            }
        }
    }
    if(judge=="PTPRel"){
        //获取当前光标所获取得item
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_PTPrel->itemFromIndex(index);
        QStandardItem* item_PTPREL=item->parent()->parent();//获取到PTP的item
        int count=0;
        for(int i=0;i<item_PTPREL->text().length();i++){
            if(item_PTPREL->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dist:DISTANCE_"){
            //当父节点为pos参数，添加在根节点的第一个位置
            QString part1="PTPRel";
            QString part2="(";
            QString part3=")";
            QString PTPREL_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
            item_PTPREL->setText(PTPREL_name);
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            if(count==0){
                //当父节点为dyn参数时，添加在根节点的第二个位置
                QString PTPREL_name=","+ui->treeView_program_correct->currentIndex().data().toString();
                int dyn_index=item_PTPREL->text().indexOf(")",1);
                QString insert=item_PTPREL->text().insert(dyn_index,PTPREL_name);
                item_PTPREL->setText(insert);
            }
        }
    }
    if(judge=="LinRel"){
        //获取当前光标所在得item
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_Linrel->itemFromIndex(index);
        QStandardItem* item_LinREL=item->parent()->parent();//获取到PTP的item
        int count=0;
        for(int i=0;i<item_LinREL->text().length();i++){
            if(item_LinREL->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dist:DISTANCE_"){
            //当父节点为pos参数，添加在根节点的第一个位置
            QString part1="LinRel";
            QString part2="(";
            QString part3=")";
            QString LINREL_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
            item_LinREL->setText(LINREL_name);
        }
        if(ui->treeView_instruction_set->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            if(count==0){
                //当父节点为dyn参数时，添加在根节点的第二个位置
                QString PTPREL_name=","+ui->treeView_program_correct->currentIndex().data().toString();
                int dyn_index=item_LinREL->text().indexOf(")",1);
                QString insert=item_LinREL->text().insert(dyn_index,PTPREL_name);
                item_LinREL->setText(insert);
            }
        }
    }
    if(judge=="StopRobot"){
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_stoprobot->itemFromIndex(index);
        QStandardItem* item_stoprobot=item->parent()->parent();
        QString temp1="StopRobot";
        QString temp2="(";
        QString temp3=")";
        QString name=temp1+temp2+ui->treeView_program_correct->currentIndex().data().toString()+temp3;
        item_stoprobot->setText(name);
    }
    if(judge=="RefRobotAxis"){
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_refrobotaxis->itemFromIndex(index);
        QStandardItem* item_refrobotaxis=item->parent()->parent();//获取到refrobotaxis的item
        int count=0;
        for(int i=0;i<item_refrobotaxis->text().length();i++){
            if(item_refrobotaxis->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="axis:ROBOTAXIS"){
            //添加的为所要回零的轴
            QString part1="RefRobotAxis";
            QString part2="(";
            QString part3=")";
            QString ref_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
            item_refrobotaxis->setText(ref_name);
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="addMoveTarget:REAL(OPT)"){
            //添加移动数值的参数
            if(count==0){
                //添加的第二个参数
                QString ref_name=","+item->child(0,1)->text();
                int add_index=item_refrobotaxis->text().indexOf(")",1);
                QString insert=item_refrobotaxis->text().insert(add_index,ref_name);
                item_refrobotaxis->setText(insert);
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //添加辅助dyn参数
            if(count==1){
                //添加第三个参数
                QString dyn_name=","+ui->treeView_program_correct->currentIndex().data().toString();
                int dyn_index=item_refrobotaxis->text().indexOf(")",1);
                QString insert=item_refrobotaxis->text().insert(dyn_index,dyn_name);
                item_refrobotaxis->setText(insert);
            }
        }
    }
    if(judge=="RefRobotAxisAsync"){
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_RefRobotAxisAsync->itemFromIndex(index);
        QStandardItem* item_RefRobotAxisAsync=item->parent()->parent();//获取到refrobotaxis的item
        QString part1="RefRobotAxisAsync";
        QString part2="(";
        QString part3=")";
        QString name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
        item_RefRobotAxisAsync->setText(name);

    }
    if(judge=="MoveRobotAxis"){
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemdoel_MoveRobotAxis->itemFromIndex(index);
        QStandardItem* item_MoveRobotAxis=item->parent()->parent();//获取到refrobotaxis的item
        int count=0;
        for(int i=0;i<item_MoveRobotAxis->text().length();i++){
            if(item_MoveRobotAxis->text()[i]==","){
                count++;
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="axis:ROBOTAXIS"){
            //添加的为所要回零的轴
            QString part1="MoveRobotAxis";
            QString part2="(";
            QString part3=")";
            QString ref_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
            item_MoveRobotAxis->setText(ref_name);
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="pos:REAL"){
            //添加目标数值的参数
            if(count==0){
                //添加的第二个参数
                QString ref_name=","+item->child(0,1)->text();
                int add_index=item_MoveRobotAxis->text().indexOf(")",1);
                QString insert=item_MoveRobotAxis->text().insert(add_index,ref_name);
                item_MoveRobotAxis->setText(insert);
            }
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="dyn:DYNAMIC_(OPT)"){
            //添加辅助dyn参数
            if(count==1){
                //添加第三个参数
                QString dyn_name=","+ui->treeView_program_correct->currentIndex().data().toString();
                int dyn_index=item_MoveRobotAxis->text().indexOf(")",1);
                QString insert=item_MoveRobotAxis->text().insert(dyn_index,dyn_name);
                item_MoveRobotAxis->setText(insert);
            }
        }
    }
    if(judge=="Ramp"){
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_Ramp->itemFromIndex(index);
        QStandardItem* item_ramp=item->parent()->parent();//获取到refrobotaxis的item
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="type:RAMPTYPE"){
            //添加到根节点的第一个位置
            QString part1="Ramp";
            QString part2="(";
            QString part3=")";
            QString Ramp_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
            item_ramp->setText(Ramp_name);
        }
        if(ui->treeView_program_correct->currentIndex().parent().data().toString()=="param:REAL(OPT)"){
            //添加到根节点的第二个位置
            int len1=item_ramp->text().length();//当前当前指令的总长
            QString part1=item_ramp->text().left(5);//第一部分：Ramp(
            QString temp=item_ramp->text().right(len1-5);
            int len2=temp.length();
            QString part2=temp.left(len2-1);//第二部分：ramp的变量名
            QString part3=ui->treeView_program_correct->currentIndex().child(0,1).data().toString();//第三部部分：ramp变量名
            QString ramp_name=part1+part2+","+part3+")";
            //判断item_PTP是否已经添加了dyn参数，若添加了，则不再settext
            QString judge=item_ramp->text();
            int count=0;
            for(int i=0;i<judge.length();i++){
                if(judge[i]==","){
                    count++;
                    break;
                }
            }
            if(count==0){
                item_ramp->setText(ramp_name);
            }
        }
    }
    if(judge=="RefSys"){
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_Refsys->itemFromIndex(index);
        QStandardItem* item_refsys=item->parent()->parent();//获取到dynovr的item
        QString part1="RefSys";
        QString part2="(";
        QString part3=")";
        QString refsys_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
        item_refsys->setText(refsys_name);
    }
    if(judge=="Tool"){
        //添加相应的参数
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_tool->itemFromIndex(index);
        QStandardItem* item_tool=item->parent()->parent();//获取waittime的item
        QString part1="Tool";
        QString part2="(";
        QString part3=")";
        QString tool_name=part1+part2+ui->treeView_program_correct->currentIndex().data().toString()+part3;
        item_tool->setText(tool_name);
    }
    //系统功能指令组
    if(judge=="WaitTime"){
        //添加相应的参数
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_waittime->itemFromIndex(index);
        QStandardItem* item_waittime=item->parent()->parent();
        QString part1="WaitTime";
        QString part2="(";
        QString part3=")";
        QString waittime_name=part1+part2+ui->treeView_program_correct->currentIndex().child(0,1).data().toString()+part3;//该行做出修改，加child
        item_waittime->setText(waittime_name);
    }
    if(judge=="Stop"){
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_stop->itemFromIndex(index);
        QStandardItem* item_stop=item->parent()->parent();
        QString temp1="Stop";
        QString temp2="(";
        QString temp3=")";
        QString name=temp1+temp2+ui->treeView_program_correct->currentIndex().data().toString()+temp3;
        item_stop->setText(name);
    }
    if(judge=="Notice"){
        //添加相应的参数
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_info->itemFromIndex(index);
        QStandardItem* item_info=item->parent()->parent();
        QString part1="Notice";
        QString part2="(";
        QString part3=")";
        QString info_name=part1+part2+item->child(0,1)->text()+part3;
        item_info->setText(info_name);
    }
    if(judge=="Warning"){
        //添加相应得参数
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_warning->itemFromIndex(index);
        QStandardItem* item_warning=item->parent()->parent();
        QString part1="Warning";
        QString part2="(";
        QString part3=")";
        QString warning_name=part1+part2+item->child(0,1)->text()+part3;
        item_warning->setText(warning_name);
    }
    if(judge=="Error"){
        //添加相应得参数
        QModelIndex index=ui->treeView_program_correct->currentIndex();
        QStandardItem* item=treemodel_error->itemFromIndex(index);
        QStandardItem* item_error=item->parent()->parent();
        QString part1="Error";
        QString part2="(";
        QString part3=")";
        QString Error_name=part1+part2+item->child(0,1)->text()+part3;
        item_error->setText(Error_name);
    }
}
//程序管理模块：点击确定按钮，修改光标所在行得内容
void Widget::on_btn_program_edit_confirm_clicked()
{
    //获取光标所在行得内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    //删除当前光标所在行得内容
    cur.removeSelectedText();
    //删除完成之后，添加新的指令内容
    QModelIndex index=ui->treeView_program_correct->currentIndex();
    QString instruction_name=index.data().toString();
    cur.insertText(instruction_name);
    int indexpage=0;
    //返回到程序管理模块得编辑界面
    switch_page_program(indexpage);
    //在开始将program_edit上的内容写入文件中前，需要将END_OFF先删除
    //若当前选中文本，需要先将文本移动到文末
    int temp=line_program_edit;//保存文本当前的行数
    cur.movePosition(QTextCursor::End);
    cur.select(QTextCursor::BlockUnderCursor);
    cur.removeSelectedText();
    //删除完行数之后，将光标移动到刚指定的行上面
    QTextBlock block=ui->program_textEdit->document()->findBlockByNumber(temp);
    ui->program_textEdit->setTextCursor(QTextCursor(block));

    //将文本数据框取出并按行排列
    QModelIndex current_index=ui->treeView_project->currentIndex();
    QStandardItem* current_item=treemodel_project->itemFromIndex(current_index);//获取当前所选的item
    QStandardItem* parent_item=current_item->parent();//获取父亲item
    QString file_path=root_prog_file+"//"+parent_item->text()+"//"+current_item->text()+".prog";//获取当前prog程序的相对路径
    QFile file(file_path);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::information(this,"错误信息","当前程序无法保存");
    }
    else{
        QTextStream out(&file);//分行写入文件
        out.setCodec(QTextCodec::codecForName("UTF-8"));//设置文件的编码格式
        out<<"{"<<"\n";
        out<<ui->program_textEdit->toPlainText()<<"\n";
        out<<"}";
    }
    //将文本写入文件之后，再在文本最后插入<<END_OFF>>
    ui->program_textEdit->append("<<END_OFF>>");
}
//程序管理模块得指令修改：设置按钮得槽函数
void Widget::on_btn_program_edit_set_clicked()
{
    //获取光标所在行得内容
    QTextCursor cur=ui->program_textEdit->textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.select(QTextCursor::LineUnderCursor);
    QString instruction_name=cur.selectedText();
    QString judge=instruction_name.left(instruction_name.indexOf("(",1));
    if(judge=="PTP"){
        instruction_set_value_correct_widget(treemodel_PTP);
    }
    if(judge=="Lin"){
        instruction_set_value_correct_widget(treemodel_Lin);
    }
    if(judge=="Circ"){
        instruction_set_value_correct_widget(treemodel_Circ);
    }
    if(judge=="PTPRel"){
        instruction_set_value_correct_widget(treemodel_PTPrel);
    }
    if(judge=="LinRel"){
        instruction_set_value_correct_widget(treemodel_Linrel);
    }
    if(judge=="RefRobotAxis"){
        instruction_set_value_correct_widget(treemodel_refrobotaxis);
    }
    if(judge=="RefRobotAxisAsync"){
        instruction_set_value_correct_widget(treemodel_RefRobotAxisAsync);
    }
    if(judge=="MoveRobotAxis"){
        instruction_set_value_correct_widget(treemdoel_MoveRobotAxis);
    }
    if(judge=="Dynovr"){
        instruction_set_value_correct_widget(treemdoel_dynovr);
    }
}
//程序管理模块：指令设置界面设置按钮的函数
void Widget::instruction_set_value_correct_widget(QStandardItemModel *b){
    QModelIndex index=ui->treeView_program_correct->currentIndex();
    QStandardItem* current_item=b->itemFromIndex(index);//当前所选的item
    QStandardItem* parent_item=current_item->parent();//父亲item
    QString Variable_name=parent_item->text();//获取变量名称
    bool isOK;
    //创建一个临时对话框，输入程序名称并且获取用户输入的程序名称
    QString program_text=QInputDialog::getText(NULL,"Input Dialog","please input Variable_Value",QLineEdit::Normal,"Variable value",&isOK);
    //需要设计算法，求出当前索引的参数的行数
    int count_parameter;
    int count_variable;
    //计算当前程序的行数
    for(int i=0;i<parent_item->rowCount();i++){
        //若当前选中的参量==父亲item第i个参量
        if(index.data().toString()==parent_item->child(i,0)->text()){
            QStandardItem* item_value=new QStandardItem(program_text);
            parent_item->setChild(i,1,item_value);
        }
    }
    //需要修改对应var文件和xml文件中变量的信息
}
