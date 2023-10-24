#ifndef MDICHILD_H
#define MDICHILD_H

#include <QMenu>
#include <QTextEdit>
#include <QWidget>

class MdiChild : public QTextEdit
{
    Q_OBJECT
private:
    QString curFile;  //当前文件路径
    bool isUntitled;  //作为当前文件是否被保存到硬盘的标志

    bool maybeSave();                              //是否需要保存
    void setCurrentFile(const QString& fileName);  //设置当前文件

protected:
    void closeEvent(QCloseEvent* event);          //关闭事件
    void contextMenuEvent(QContextMenuEvent* e);  // 右键菜单事件

public:
    explicit MdiChild(QWidget* parent = 0);
    void newFile();                            //新建文件
    bool loadFile(const QString& fileName);    //加载文件
    bool save();                               //保存操作
    bool saveAs();                             //另存为操作
    bool saveFile(const QString& fileName);    //保存文件
    QString userFriendlyCurrentFile();         //提取文件名
    QString currentFile() { return curFile; }  //返回当前文件路径
private slots:
    void documentWasModified();  //文档被更改时，窗口显示更改状态标志
};

#endif  // MDICHILD_H
