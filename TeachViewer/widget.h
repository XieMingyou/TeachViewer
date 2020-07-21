#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QStandardItemModel>
#include<QItemSelection>
#include<QStack>
#include<QMap>
#include<iostream>
using namespace  std;
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    //变量管理模块的公有成员函数
    void set_program_range_variable_xml_update();//切换到变量管理界面时，解析程序变量作用域xml文件，将解析的信息呈现在treeview_variable上
    void set_project_range_variable_xml_update();//切换到变量管理模块，解析项目变量作用域xml文件，将解析的信息呈现在treeview_variable上
    void set_all_range_variable_xml_update();//解析全局变量作用域xml文件，将解析的信息呈现在treeview_variable上
    void set_detetmine_type_all_range_variable_xml_update(QString type);//解析全局变量作用域中特定的数据类型，将解析的信息呈现在treeview_variable中
    void set_determine_type_project_range_variable_xml_update(QString type);//解析项目变量作用域中特定的数据类型，将解析的信息呈现在treeview_variable中
    void set_detemine_type_program_range_variable_xml_update(QString type);//解析程序变量作用域中特定的数据类型，将解析的信息呈现在treeview_variable中
    void set_variavle_charge_widget();//布局变量管理模块的基本界面
    void set_different_variable_type_parameter(QStandardItem* item_variable,QString type_name);//根据不同的数据类型，插入不同的参数到变量节点中
    void set_only_one_parameter(QStandardItem* item_variable,QString name);//插入单一参数的变量
    void set_TOOL_parameter(QStandardItem* item_variable);//插入该数据类型的参数
    void set_CARTREFSYS_parameter(QStandardItem* item_variable);//插入该数据类型的参数
    void set_OVERLAP_parameter(QStandardItem* item_variable);//插入该参数类型的参数
    void generate_all_range_variable_xml_file();//生成全局变量的xml文件
    void generate_project_range_variable_xml_file();//生成项目变量的xml文件
    void generate_program_range_variable_xml_file();//生成程序变量的xml文件
    void generate_program_var_file();//将变量插入到var文件中
    QString get_different_format_var_string(QStandardItem* root,QString type);//返回不同数据类型插入var文件的变量格式
    void insert_format_value_to_var_file(QString path,QString value);//将当前要插入的指令插入var文件中
    void delete_current_choose_variable_var(QString path,QString delete_name);//前往相应程序文件，将相应的指令删除
    void delete_current_choose_variable_xml(QString path,QString delete_name);//前往相应程序文件，将相应的指令删除
    void rename_current_choose_variable_var(QString path,QString old_name,QString new_name);// 前往相应程序文件，将相应的指令重命名
    void set_new_variable_to_different_range_xml_file(QStandardItem* item,QString name);//将新建的变量名存放到相应的xml文件中
    void set_new_variable_to_different_range_var_file(QStandardItem* item);//将新建的变量存放到相应的var文件中
    void delete_different_range_variable_xml(QStandardItem* item);//根据程序路径，删除对应的xml文件中对应变量的信息
    void delete_different_range_variable_var(QStandardItem* item);//根据程序路径，删除对应的var文件中对应变量的信息
    void rename_different_range_variable_to_xml_file(QStandardItem* item,QString old_name,QString new_name);//更新节点信息进入xml文件中
    void update_set_value_to_xml_file(QStandardItem* item);//将设置的变量数值在对应的xml文件中进行刷新
    bool flag_judge_variable_is_used_or_not(QString path,QString name);//判断某个变量在程序中是否用到,用了返回true,否则为false
    void update_xml_determined_to_variable_type(int a);//根据变量类型，重新读取节点到模型当中

    //项目管理模块下的公有成员函数
    void set_project_charge_widget();//设置项目管理模块的基本界面
    void project_prase();//解析文件显示在treeview_project上
    void project_prase2();//解析文件显示在treeview_project上的第二步
    void set_project_program_status(QString);//设置项目和程序的状态，加载，打开或者终止
    void set_instruction_choose();//布局指令选择界面的函数
    void set_current_instruction();//选中指令，点击确定之后进入对应的指令设置界面
    void set_instruction_set();//布局指令设置界面的函数
    void set_xml_to_PTP_Lin_Circ(QStandardItem* root);//解析xml文件,看文件中是否有节点，即是否有变量，若有的话，需要加到相应的参数节点之下供用户选择
    void set_project_range_xml_to_PTP_Lin_Circ(QStandardItem* root);//解析xml文件，将项目作用域的变量节点写到对应的模型中
    void set_all_range_xml_to_PTP_Lin_Circ(QStandardItem* root);//解析xml文件，将全局作用域的变量节点写到对应的模型中
    void set_different_range_xml_to_refrobot_model(QStandardItem* root,QString path);//解析变量xml文件，显示到refrobot的模型中
    void set_different_range_xml_to_refrobotAxis_model(QStandardItem* root,QString path);//解析变量xml文件，显示到refrobotAxis的模型中
    void set_dynamic_variable_to_dyn_model(QStandardItem* root,QString path);//解析xml文件，显示到dyn模型上
    void set_IF_variable_to_dynovr_model(QStandardItem* root,QString path);//解析xml文件，显示到dynovr模型中
    void set_IF_variable_to_Ramp_model(QStandardItem* root,QString path);//解析xml文件，显示到Ramp模型中
    void set_Variable_to_Refsys_model(QStandardItem* root,QString path);//解析相应的数据类型到RefSys的模型节点中
    void set_variable_to_tool_model(QStandardItem* root,QString path);//解析相应的数据类型的tool的模型节点中
    void set_variable_to_waittime_model(QStandardItem* root,QString path);//解析相应的数据类型到waittime的模型节点中
    void set_string_variable_to_info_model(QStandardItem* root,QString path);//解析相应的数据类型到info的模型节点中
    void deal_StopRobot_instruction();//处理stoprobot指令的函数
    void deal_stop_instruction();//处理stop指令的函数
    void deal_fuzhi_instruction();//处理赋值指令的函数
    void deal_zhushi_instruction();//处理注释指令的函数
    void deal_call_instruction();//处理call指令的函数
    void deal_wait_instruction();//处理wait指令的函数
    void deal_if_then_instruction();//处理IF指令的函数
    void deal_different_if_then_instruction(QString a,QString b);//处理所选的IF指令设置界面
    void deal_while_instruction();//处理while指令的函数
    void deal_loop_instruction();//处理loop指令的函数
    void deal_run_instruction();//处理run指令的函数
    void deal_return_instruction();//处理return指令的函数
    void deal_goto_instruction();//处理goto指令的函数
    void insert_instruction_to_program_edit(QString instruction_name);//将流程控制指令插入program_edit当中，并保存文件
    void update_program_status();//刷新保存程序状态的xml文件
    void parse_program_status();//解析xml文件，读取上次退出时程序的状态
    void base_program_status_to_operate(QString project_name,QString program_name,QString status);//根据treeview_project上的状态，对程序进行相应的加载或打开操作
    void set_all_project_range_to_new_var_file(QString project_path,QString program_path);//所创建的新的程序文件，将全局var和该程序对应的项目var文件中的变量信息写到该程序var文件中
    void correct_instruction_to_program_edit(QString name);//程序管理模块，修改指令按钮，根据修改得参数名称，修改stoprobot指令
    //程序管理模块的公有成员函数
    void instruction_set_value(QStandardItemModel* b);//指令设置窗口设置参数值的函数
    void instruction_set_value_correct_widget(QStandardItemModel* b);//指令修改窗口设置参数值得函数
    void set_instruction_to_program_edit();//程序管理模块中指令设置界面的确定按钮：将指令插入文本中
    void set_parameter_to_var_file();//确认按钮，插入变量信息到var文件
    void set_pos_AXIPOS_parameter(QStandardItem* item_variable);//设置pos_AXISPOS的指令插入
    void set_pos_CARTPOS_parameter(QStandardItem* item_variable);//设置pos_CARTPOS的指令插入
    void set_dyn_parameter(QStandardItem* tem_variable);//设置dyn的指令
    QString get_var_parameter_format_value(QString parameter_name,QStandardItem* root,int parameter_number,int parameter_type);//通过该函数来获取所要添加的var文件的变量格式的名称
    void save_parameter_to_var_file(QString parameter_name);//将变量信息保存到var文件中
    void set_parameter_to_xml_file();//确认按钮，插入变量信息到xml文件
    void set_different_instruction_parameter_to_xml_file(QStandardItem* root);//将不同的指令变量信息插入到xml文件中
    void set_different_instruction_parameter_to_project_range_xml_file(QStandardItem* root);//该函数生成项目作用域的xml文件
    void set_different_instruction_parameter_to_all_range_xml_file(QStandardItem* root);//该函数生成全局作用域的xml文件
    void deal_GOTO_LABEL_instruction();//处理该LABEL指令的函数
    void save_program_to_prog_file();//将指令存放到当前加载程序的prog文件中
    void set_new_variable_from_instruction_set_widget_to_different_range_xml_file(QStandardItem* item,QString name);//将指令设置中新建得变量添加到xml文件中
    void set_new_variable_from_instruction_set_widget_to_different_range_var_file(QStandardItem* item,QString name);//将指令设置中新建得变量添加到var文件中
    //位置管理模块的公有成员函数
    void set_coordinate_charge_widget();//布局位置管理模块的基本界面
    QStandardItemModel *tablemodel_coordinate;//位置管理模块的模型
    int coordinateShow;//位置管理模块中选中要显示的坐标系标志，1：伺服，2：关节，3：世界
    //信息报告管理模块得共有成员函数
    void set_information_charge_widget();//布局信息报告管理模块基本界面
    QStandardItemModel* tablemodel_information;//信息报告模块的模型

public slots:

private slots:
    //七个模块切换按钮的槽函数
    void on_btn_user_diy_clicked();
    void on_btn_config_charge_clicked();
    void on_btn_variable_charge_clicked();
    void on_btn_project_charge_clicked();
    void on_btn_program_charge_clicked();
    void on_btn_coordinate_charge_clicked();
    void on_btn_information_report_charge_clicked();
    //菜单栏上read按钮的槽函数
    void on_btn_read_information_clicked();
    //变量管理模块的槽函数
    void slt_variable_new_variable();//新建变量
    void slt_variable_delete_variable();//删除变量
    void slt_variable_copy_variable();//复制变量
    void slt_variable_paste_variable();//粘贴变量
    void slt_variable_cut_variable();//剪切变量
    void slt_variable_rename_variable();//重命名变量
    void slot_treeview_variable_CurrentChanged(const QModelIndex& t, const QModelIndex& previous);//选中treeview_variable选项时，触发该槽函数
    void on_btn_variable_set_clicked();//变量管理模块设置按钮的槽函数
    void slt_deal_variable_type(int a);//处理下拉框不同选项的槽函数

    //项目管理模块下的槽函数
    void on_btn_project_update_clicked();//项目管理模块：刷新按钮的槽函数
    void on_btn_project_load_clicked();//项目管理模块：加载按钮的槽函数
    void slt_project_file_new_project();//新建项目槽函数
    void slt_project_file_new_program();//新建程序槽函数
    void slt_project_file_delete();//删除槽函数
    void slt_project_file_rename();//重命名槽函数
    void on_btn_project_open_clicked();//打开按钮的槽函数
    void on_btn_project_stop_clicked();//终止按钮的槽函数
    void slot_treeview_program_CurrentChanged(const QModelIndex& t, const QModelIndex& previous);//选中treeview_project选项时，触发该槽函数

    //程序管理模块下的槽函数
    void onCurrentLineHighLight();//该槽函数使程序呈现高亮状态
    int program_text_cursorPositionChaned();//该槽函数会返回光标所在的行号，为后续新建指令做准备
    //程序管理模块中新建按钮的槽函数：切换到指令选择界面
    void on_btn_program_new_clicked();
    //程序管理模块中：指令选择界面返回按钮，切换回程序编辑界面
    void on_btn_instruction_back_clicked();
    //程序管理模块中：指令设置界面返回按钮：切换为指令选择界面
    void on_btn_instruction_back_2_clicked();
    //程序管理模块：指令选择界面确定按钮，切换到指令设置界面
    void on_btn_instruction_confirm_clicked();
    //程序管理模块：指令设置界面确定按钮，切换到程序编辑界面
    void on_btn_instruction_confirm_2_clicked();
    //程序管理模块：指令设置界面新建按钮选项的槽函数
    void slt_new_Variable();
    void slt_program_edit_copy();//程序的复制
    void slt_program_edit_paste();//程序的编辑
    void slt_program_edit_cut();//程序的剪切
    void slt_program_edit_delete();//程序的删除
    void slt_program_edit_annotate();//程序的注释
    void slt_program_edit_unannotate();//程序的取消注释
    void slot_treeview_instruction_choose_CurrentChanged(const QModelIndex& t, const QModelIndex& previous);//选中treeview_project选项时，触发该槽函数
    void slot_treeview_instruction_set_CurrentChanged(const QModelIndex& t, const QModelIndex& previous);//选中treeview_project选项时，触发该槽函数
    void slot_treeview_instruction_program_correct_CurrentChanged(const QModelIndex& t, const QModelIndex& previous);//选中treeview_program_correct选项时，出发该槽函数
    //程序管理模块：编辑按钮的槽函数
    void on_btn_program_edit_clicked(bool checked);
    void on_btn_program_hong_clicked();//程序管理模块:指令修改按钮的槽函数
    //程序管理模块：指令设置界面添加按钮的槽函数
    void on_btn_instruction_add_clicked();
    //程序管理模块：指令设置界面设置按钮的槽函数
    void on_btn_instruction_set_clicked();

    //位置管理模块的槽函数
    //点动速度的槽函数
    void slt_inching_speed_01();
    void slt_inching_speed_1();
    void slt_icching_speed_10();
    void slt_inching_speed_25();
    void slt_inching_speed_50();
    void slt_inching_speed_100();
    //点动的槽函数
    void slt_inching_tool();
    void slt_inching_world();
    void slt_inching_axis();


    //界面底部按钮的槽函数
    //伺服上电按钮槽函数
    void on_btn_pwr_clicked();
    //开始按钮的槽函数
    void on_btn_start_clicked();
    void on_btn_start_pressed();
    void on_btn_start_released();
    //停止按钮的槽函数
    void on_btn_stop_clicked();
    //切换坐标系按钮的槽函数
    void on_btn_jog_clicked();
    //单步/连续模式的槽函数
    void on_btn_step_clicked();
    //手动/自动模式的槽函数
    void on_btn_hand_auto_clicked();
    //V+按钮槽函数：是机器人速度加速
    void on_btn_V_up_clicked();
    //V-按钮槽函数：使机器人速度减速
    void on_btn_V_down_clicked();
    //手动示教时，-+按钮的槽函数
    void on_btn_coordinate1_down_pressed();
    void on_btn_coordinate1_down_released();
    void on_btn_coordinate1_up_pressed();
    void on_btn_coordinate1_up_released();
    void on_btn_coordinate2_down_pressed();
    void on_btn_coordinate2_down_released();
    void on_btn_coordinate2_up_pressed();
    void on_btn_coordinate2_up_released();
    void on_btn_coordinate3_down_pressed();
    void on_btn_coordinate3_down_released();
    void on_btn_coordinate3_up_pressed();
    void on_btn_coordinate3_up_released();
    void on_btn_coordinate4_down_pressed();
    void on_btn_coordinate4_down_released();
    void on_btn_coordinate4_up_pressed();
    void on_btn_coordinate4_up_released();
    void on_btn_coordinate5_down_pressed();
    void on_btn_coordinate5_down_released();
    void on_btn_coordinate5_up_pressed();
    void on_btn_coordinate5_up_released();
    void on_btn_coordinate6_down_pressed();
    void on_btn_coordinate6_down_released();
    void on_btn_coordinate6_up_pressed();
    void on_btn_coordinate6_up_released();
    void on_btn_coordinate_servo_clicked();
    void on_btn_coordinate_joint_clicked();
    void on_btn_coordinate_world_clicked();
    void on_btn_variable_clear_clicked();


    void on_btn_program_edit_back_clicked();

    void on_btn_program_edit_add_clicked();

    void on_btn_program_edit_confirm_clicked();

    void on_btn_program_edit_set_clicked();

private:
    Ui::Widget *ui;
    int count_coordinate = 0;//切换坐标系时候需要用的索引
    int count_auto = 0;//切换手动，自动，远程模式需要用的索引
    int hasLoad = 0; //是否已经加载程序，0：未加载，1：已加载
    void switch_page(int a);//通过调用该函数来切换界面
    void switch_page_program(int a);//通过调用该函数来在程序管理模块中切换界面
    //变量管理模块的控件
    QStandardItemModel* treemodel_variable;//变量管理模块的模型
    QStandardItem* root_variable;//变量管理模块模型的根节点
    QAction* variable_new;//变量下的新建控件
    QAction* variable_delete;//变量下的删除控件
    QAction* variable_copy;//变量下的复制控件
    QAction* variable_paste;//变量下的粘贴控件
    QAction* variable_cut;//变量下的剪切控件
    QAction* variable_rename;//变量下的重命名控件
    QMap<QString,QString> type_attribute_variable;//存放变量名和对应的数据类型
    QMap<QString,QString>type_range_variable;//存储变量名和对应的变量作用域
    QStandardItem* copy_item;//复制的item
    //项目管理模块的控件
    QStandardItemModel* treemodel_project;
    QStandardItem* item_root_project;
    QString root_path="prog_var_xml";//根路径
    QString root_prog_file=root_path+"//prog";//项目及程序文件的根目录
    QString root_var_file=root_path+"//var";//var文件的根目录
    QString root_xml_file=root_path+"//xml";//xml文件的根目录
    QAction* file_new_project;
    QAction* file_new_program;
    QAction* file_delete;
    QAction* file_rename;
    QAction* file_copy;
    QAction* file_cut;
    QAction* file_paste;
    QStack<QString> label_information_text;//用来显示label_information上的信息

    //程序管理模块的数据成员
    QString program_edit_copy_cut_text;//复制、剪切的程序文本
    QStandardItemModel* treemodel_instruction_choose;//treeview_instruction_choose的模型
    QStandardItem* item_root_instruction_choose;//treeview_instruction_choose的根item
    QStandardItemModel* treemodel_PTP;//PTP指令的模型
    QStandardItem* root_PTP;//PTP指令的根节点
    QStandardItemModel* treemodel_Lin;//Lin指令的模型
    QStandardItem* root_Lin;//Lin指令的根节点
    QStandardItemModel* treemodel_Circ;//Circ指令的模型
    QStandardItem* root_Circ;//Circ指令的很节点
    QStandardItemModel* treemodel_PTPrel;//PTP插补指令的模型
    QStandardItem* root_PTPrel;//PTP插补指令的根节点
    QStandardItemModel* treemodel_Linrel;//LinRel线性插补的模型
    QStandardItem* root_LinRel;//LinRel插补指令的根节点
    QStandardItemModel* treemodel_refrobotaxis;//refrobotaxis的模型
    QStandardItem* root_refrobotaxis;//refrobotaxis的根节点
    QStandardItemModel* treemodel_RefRobotAxisAsync;
    QStandardItem* root_RefRobotAxisAsync;
    QStandardItemModel* treemdoel_MoveRobotAxis;//对机器人单一轴移动的模型
    QStandardItem* root_MoveRobotAxis;//单一轴移动的根节点
    QStandardItemModel* treemodel_dyn;
    QStandardItem* root_dyn;
    QStandardItemModel* treemdoel_dynovr;
    QStandardItem* root_dynovr;
    QStandardItemModel* treemodel_Refsys;
    QStandardItem* root_Refsys;
    QStandardItemModel* treemodel_Ramp;
    QStandardItem* root_Ramp;
    QStandardItemModel* treemodel_tool;
    QStandardItemModel* treemodel_waittime;
    QStandardItem* root_waittime;
    QStandardItem* root_tool;
    QStandardItemModel* treemodel_info;
    QStandardItem* root_info;
    QStandardItemModel* treemodel_warning;
    QStandardItem* root_warning;
    QStandardItemModel* treemodel_error;
    QStandardItem* root_error;
    QStandardItemModel* treemodel_stoprobot;
    QStandardItem* root_stoprobot;
    QStandardItemModel* treemodel_stop;
    QStandardItem* root_stop;
    QMap<QString,QString> type_attribute;//存放新建的变量名和对应的数据类型
    QMap<QString,QString>type_range;//存储新建的变量名和对应的变量作用域
    int line_program_edit;//当前光标所在的行数

    //位置管理模块的控件
    QAction* inching_speed_01;//显示0.1inc的速度
    QAction* inching_speed_1;//显示1的速度
    QAction* inching_speed_10;//显示10%的速度
    QAction* inching_speed_25;//显示25%的速度
    QAction* inching_speed_50;//显示50的速度
    QAction* inching_speed_100;//显示100%的速度
    QAction* inching_tool;//工具选项
    QAction* inching_world;//世界选项
    QAction* inching_axis;//轴选项

protected:
    void timerEvent(QTimerEvent* event);//创建时间事件处理函数
};
#endif // WIDGET_H
