# Wordx C++ Library

该库的作用是在代码中实现对word文档(.docx格式)的一些操作
- [Excelx](https://github.com/lixuan998/Excelx)


## 功能

 - [x] 替换文字标签
 - [x] 替换图片标签
 - [x] 循环插入表格
 - [x] 循环替换

## 依赖
- OpenCV (建议4.5.5版本)
- Qt (建议5.12.12版本)
- CMake (3.5以上版本)

## 安装
- 下载本库
- 将本库文件夹移动到你的项目目录中
- 在你的项目的CMakeLists.txt中添加 
``` CMake
  set(XML_MODEL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wordx/xml_models)
  set(XML_MODEL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/xml_models)

  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory  "${XML_MODEL_SOURCE_DIR}" "${XML_MODEL_BINARY_DIR}")

  add_subdirectory(wordx)

  target_include_directories(${YOUR_PROJECT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/wordx/include)#也可以不用这行，但是在代码中头文件应改成#include"../include/wordx.h"

  target_link_libraries(${YOUR_PROJECT} Wordx)#一般放在最后
```


##使用

``` C++
#include "wordop.h"

//该函数演示了如何将预设的标签替换成给定文本
void how_to_replace_text(WordOp &op);

//该函数演示了如何使用Info类将预设的标签替换成给定文本
void how_to_replace_text_with_info(WordOp &op);

//该函数演示了如何替换预设标签替换成给定图片, 图片以cv::Mat的形式传入
void how_to_replace_images_given_by_mat(WordOp &op);

//该函数演示了如何替换预设标签替换成给定图片, 图片以图片路径的形式传入
void how_to_replace_images_given_by_path(WordOp &op);

//该函数演示了如何使用循环添加功能
void how_to_add_informations_recursively(WordOp &op);

//该函数演示了如何循环插入表格
void how_to_add_table_rows(WordOp &op);

/*-----------------------------------
| demo 的运行方法：                   |
| ./wordx <模板文件路径>              |
| 如： ./wordx ../standard_model.docx|
-----------------------------------*/

int main(int argc, char *argv[])
{
    //待处理文件地址
    QString word_path;
    if(argc < 2)
    {
        word_path = "standard_model.docx";
    }
    else word_path = argv[1];

    //生成的文件地址
    QString des_path = "../generation_of_standard_model.docx";

    //创建WordOp类的实例，可在构造函数中制定打开的.docx文档的路径或者在open()方法中指定
    WordOp op(word_path, des_path);

    //打开读取的待处理docx文件
    op.open();

    how_to_replace_text_with_info(op);

    how_to_replace_images_given_by_mat(op);

    how_to_replace_images_given_by_path(op);

    how_to_add_informations_recursively(op);

    how_to_add_table_rows(op);

    //关闭docx文件
    op.close();

    return 0;
}

void how_to_replace_text(WordOp &op)
{
    std::vector<QString> marks{"${STL_SN}",
                                   "${STL_CRAT_TME}", 
                                   "${STL_LEN}", 
                                   "${STL_WID}", 
                                   "${STL_SKNESS}",
                                   "${STL_DFCT_TP}"};

    std::vector<QString> replace_with{"1000001",
                                          "2023年2月9日",
                                          "10",
                                          "20",
                                          "22",
                                          "污渍"};

    QString rep_str = "";
    /*----------------------------------------------
    | 第一个参数为vector<string>类型，表示自定义的标签集合|
    | 第一个参数为vector<string>类型，表示自定义的文本集合|
    | 标签集合和文本集合里的元素是一一对应的              |
    ----------------------------------------------*/
    op.replaceText(marks, replace_with);
}

void how_to_replace_text_with_info(WordOp &op)
{
    Info info;
    info.addInfo("${STL_SN}", "1000001");
    info.addInfo("${STL_CRAT_TME}", "2023年2月9日");
    info.addInfo("${STL_LEN}", "10");
    info.addInfo("${STL_WID}", "20");
    info.addInfo("${STL_SKNESS}", "22");
    info.addInfo("${STL_DFCT_TP}", "污渍");
    op.replaceText(info);
}

void how_to_replace_images_given_by_mat(WordOp &op)
{
    std::vector<QString> marks{"${ST_UP_SURFACE}"};
    std::vector<cv::Mat> replace_mat_images{cv::imread("./demo_files/test1.jpg")};

    /*---------------------------------------------------
    | 第一个参数为vector<string>类型，表示自定义的标签集合     |
    | 第二个参数为vector<Mat>类型，表示需要替换上去的图片集合   |
    | 标签集合和图片集合里的元素是一一对应的                   |
    ---------------------------------------------------*/
    op.replaceImageFromMat(marks, replace_mat_images);
}

void how_to_replace_images_given_by_path(WordOp &op)
{
    std::vector<QString> marks{"${ST_DOWN_SURFACE}"};
    std::vector<QString> replace_image_pathes{"./demo_files/test2.jpg"};

    /*-------------------------------------------------------
    | 第一个参数为vector<string>类型，表示自定义的标签集合         |
    | 第二个参数为vector<string>类型，表示需要替换上去的图片路径集合 |
    | 标签集合和图片路径集合里的元素是一一对应的                    |
    -------------------------------------------------------*/
    op.replaceImage(marks, replace_image_pathes);
}

void how_to_add_informations_recursively(WordOp &op)
{
    QString defects[]{"污渍", "划痕", "氧化", "橡胶", "苹果"};
    QString surfaces[]{"上表面", "下表面", "上表面", "下表面", "上表面"};
    QString xs[]{"10", "20", "30", "40", "50"};
    QString ys[]{"1000", "2000", "3000", "4000", "5000"};
    QString sns[]{"1", "2", "3", "4", "5"};
    /*-----------------------------------
    | 此处的Info为我定义的类               |
    | Info类包含了将要被替换的文本或图片的信息|
    | Info类主要由两个map构成              |
    | 一个是标签到替换文本的map             |
    | 一个是标签到替换图片的map             |
    -----------------------------------*/
    Info info;
    std::vector<Info> rec_infos;
    std::vector<int> rec_indexes{1, 2};
    for(int i = 0; i < 5; ++ i)
    {
        /*-----------------------------------------------------------------
        | Info类使用clear()函数清空                                          |
        | Info类的addInfo方法向Info类实例中添加 “标签-文本” 或 “标签-图片” 的键值对|
        | 其中图片以cv::Mat的形式                                             |
        ------------------------------------------------------------------*/
        info.clear();
        info.addInfo("${LP_SN}", sns[i]);
        info.addInfo("${LP_DEFECT}", defects[i]);
        info.addInfo("${LP_SURFACE}", surfaces[i]);
        info.addInfo("${LP_X}", xs[i]);
        info.addInfo("${LP_Y}", ys[i]);
        info.addInfo("${LP_IMAGE}", cv::imread("./demo_files/steel1.jpeg"));
        //for(int j = 0; j < 1; ++ j) rec_infos.push_back(info);
        rec_infos.push_back(info);
    }
    /*---------------------------------------------------------------------------------------------
    | 考虑到一个模板中可能存在n个循环体，所以第一个参数为一个vector<int>类型，表示需要被替换的循环体， 计数从1开始|
    | 第二个参数为vector<Info>类型， 表示替换的图片和文本信息                                            |
    | 注意，如果要同时替换多个循环体，则这两个循环体的标签集应相同                                          |
    ---------------------------------------------------------------------------------------------*/
    op.addInfoRecursive(rec_indexes, rec_infos);
}

void how_to_add_table_rows(WordOp &op)
{
    //关于Info的介绍请看 “how_to_add_informations_recursively“ 中的介绍
    Info info;
    std::vector<Info> table_infos;
    std::vector<int> table_indexes{1};

    QString arg1[]{"1", "2", "3", "4", "5"};
    QString arg2[]{"污渍", "划痕", "氧化", "橡胶", "苹果"};
    QString arg3[]{"1000", "2000", "3000", "4000", "5000"};

    for(int i = 0; i < 5; ++ i)
    {
        info.clear();
        info.addInfo("${TB_ARG1}", arg1[i]);
        info.addInfo("${TB_ARG2}", arg2[i]);
        info.addInfo("${TB_ARG3}", arg3[i]);
        info.addInfo("${TB_ARG4}", cv::imread("./demo_files/steel1.jpeg"));
        for(int j = 0; j < 1; ++ j) table_infos.push_back(info);
        //table_infos.push_back(info);
    }
    /*-----------------------------------------------------------------------------------------------------
    | 考虑到一个模板中可能存在n个待插入表格，所以第一个参数为一个vector<int>类型，表示需要被循环插入的表格， 计数从1开始   |
    | 若需同时插入k个表格，则第一个参数中的下标集顺序应为从后往前，例如要替换插入第一个和第二个表格，则参数中的顺序为 "2， 1"|
    | 第二个参数为vector<Info>类型， 表示替换的图片和文本信息                                                    |
    | 注意，如果要同时替换插入多个表格，则这两个表格的标签集应相同                                                  |
    ----------------------------------------------------------------------------------------------------*/                                                
    op.addTableRows(table_indexes, table_infos);
}
```

## API
```
#include "wordx.h"
```
- int replaceText(QString mark, QString replaced_text, QString &rep_str)
- int replaceText(Info info);
- int replaceText(std::vector<QString> marks, std::vector<QString> replaced_texts, QString &rep_str);
- int replaceText(std::vector<QString> marks, std::vector<QString> replaced_texts);
- int replaceText(QString mark_file, QString replaced_text_file);
- int replaceImage(std::vector<QString> marks, std::vector<QString> replace_image_paths);
- int replaceImageFromMat(std::vector<QString> marks, std::vector<cv::Mat> replace_mat_images);
- int replaceImageFromFile(QString mark_file_path, QString replace_image_file_path);
- int addInfoRecursive(std::vector<int> indexs, std::vector<Info> &infos);
- int addTableRows(std::vector<int> indexs, std::vector<Info> &infos);

