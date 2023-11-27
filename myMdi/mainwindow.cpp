#include "mainwindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>

#include "mdichild.h"
#include "ui_mainwindow.h"

// 活动窗口
MdiChild* MainWindow::activeMdiChild()
{
    // 如果有活动窗口，则将其内的中心部件转换为 MdiChild 类型
    if (QMdiSubWindow* activeSubWindow = ui->mdiArea->activeSubWindow())
        return qobject_cast<MdiChild*>(activeSubWindow->widget());
    // 没有活动窗口，直接返回 0
    return 0;
}

// 查找子窗口
QMdiSubWindow* MainWindow::findMdiChild(const QString& fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    // 利用foreach语句遍历子窗口列表，如果其文件路径和要查找的路径相同，则返回该窗口
    foreach (QMdiSubWindow* window, ui->mdiArea->subWindowList())
    {
        MdiChild* mdiChild = qobject_cast<MdiChild*>(window->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return window;
    }
    return 0;
}

// 读取窗口设置
void MainWindow::readSettings()
{
    QSettings settings("uestc_xiye", "myMdi");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}

// 写入窗口设置
void MainWindow::writeSettings()
{
    QSettings settings("uestc_xiye", "myMdi");
    // 写入位置信息
    settings.setValue("pos", pos());
    // 写入大小信息
    settings.setValue("size", size());
}

// 初始化窗口
void MainWindow::initWindow()
{
    setWindowTitle(tr("多文档编辑器"));
    // 在工具栏上单击鼠标右键时，可以关闭工具栏
    ui->mainToolBar->setWindowTitle(tr("工具栏"));
    // 当多文档区域的内容超出可视区域后，出现滚动条
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->statusBar->showMessage(tr("欢迎使用多文档编辑器"));
    QLabel* label = new QLabel(this);
    label->setFrameStyle(QFrame::Box | QFrame::Sunken);
    label->setText(tr("<a href=\"https://blog.csdn.net/ProgramNovice\">CSDN 博客</a>"));
    // 标签文本为富文本
    label->setTextFormat(Qt::RichText);
    // 可以打开外部链接
    label->setOpenExternalLinks(true);
    ui->statusBar->addPermanentWidget(label);
    ui->actionNew->setStatusTip(tr("创建一个文件"));
    // 设置其他动作的状态提示
    ui->actionOpen->setStatusTip(tr("打开一个已经存在的文件"));
    ui->actionSave->setStatusTip(tr("保存文档到硬盘"));
    ui->actionSaveAs->setStatusTip(tr("以新的名称保存文档"));
    ui->actionExit->setStatusTip(tr("退出应用程序"));
    ui->actionUndo->setStatusTip(tr("撤销先前的操作"));
    ui->actionRedo->setStatusTip(tr("恢复先前的操作"));
    ui->actionCut->setStatusTip(tr("剪切选中的内容到剪贴板"));
    ui->actionCopy->setStatusTip(tr("复制选中的内容到剪贴板"));
    ui->actionPaste->setStatusTip(tr("粘贴剪贴板的内容到当前位置"));
    ui->actionClose->setStatusTip(tr("关闭活动窗口"));
    ui->actionCloseAll->setStatusTip(tr("关闭所有窗口"));
    ui->actionTile->setStatusTip(tr("平铺所有窗口"));
    ui->actionCascade->setStatusTip(tr("层叠所有窗口"));
    ui->actionNext->setStatusTip(tr("将焦点移动到下一个窗口"));
    ui->actionPrevious->setStatusTip(tr("将焦点移动到前一个窗口"));
    ui->actionAbout->setStatusTip(tr("显示本软件的介绍"));
    ui->actionAboutQt->setStatusTip(tr("显示Qt的介绍"));
}

// 关闭事件
void MainWindow::closeEvent(QCloseEvent* event)
{
    // 先执行多文档区域的关闭操作
    ui->mdiArea->closeAllSubWindows();
    // 如果还有窗口没有关闭，则忽略该事件
    if (ui->mdiArea->currentSubWindow())
    {
        event->ignore();
    }
    else
    {
        // 在关闭前写入窗口设置
        writeSettings();
        event->accept();
    }
}

// 构造函数
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建间隔器动作并在其中设置间隔器
    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    // 更新菜单
    updateMenus();
    // 当有活动窗口时更新菜单
    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMenus()));

    // 创建信号映射器
    windowMapper = new QSignalMapper(this);
    // 映射器重新发送信号，根据信号设置活动窗口
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
    // 更新窗口菜单，并且设置当窗口菜单将要显示的时候更新窗口菜单
    updateWindowMenu();
    connect(ui->menuW, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));
    // 初始窗口时读取窗口设置信息
    readSettings();
    // 初始化窗口
    initWindow();
}

// 析构函数
MainWindow::~MainWindow() { delete ui; }

// 新建文件菜单
void MainWindow::on_actionNew_triggered()
{
    // 创建 MdiChild
    MdiChild* child = createMdiChild();
    // 新建文件
    child->newFile();
    // 显示子窗口
    child->show();
}

// 打开文件菜单
void MainWindow::on_actionOpen_triggered()
{
    // 获取文件路径
    QString fileName = QFileDialog::getOpenFileName(this);
    // 如果路径不为空，则查看该文件是否已经打开
    if (!fileName.isEmpty())
    {
        QMdiSubWindow* existing = findMdiChild(fileName);
        // 如果已经存在，则将对应的子窗口设置为活动窗口
        if (existing)
        {
            ui->mdiArea->setActiveSubWindow(existing);
            return;
        }
        // 如果没有打开，则新建子窗口
        MdiChild* child = createMdiChild();
        if (child->loadFile(fileName))
        {
            ui->statusBar->showMessage(tr("打开文件成功"), 2000);
            child->show();
        }
        else
        {
            child->close();
        }
    }
}

// 保存菜单
void MainWindow::on_actionSave_triggered()
{
    if (activeMdiChild() && activeMdiChild()->save())
        ui->statusBar->showMessage(tr("文件保存成功"), 2000);
}

// 另存为菜单
void MainWindow::on_actionSaveAs_triggered()
{
    if (activeMdiChild() && activeMdiChild()->saveAs())
        ui->statusBar->showMessage(tr("文件保存成功"), 2000);
}

// 退出菜单
void MainWindow::on_actionExit_triggered()
{
    // 这里的 qApp 是 QApplication 对象的全局指针
    qApp->closeAllWindows();
    // 这行代码相当于 QApplication::closeAllWindows()，等效于 qApp->quit() 和 qApp->exit(0)
}

// 撤销菜单
void MainWindow::on_actionUndo_triggered()
{
    if (activeMdiChild())
        activeMdiChild()->undo();
}

// 恢复菜单
void MainWindow::on_actionRedo_triggered()
{
    if (activeMdiChild())
        activeMdiChild()->redo();
}

// 剪切菜单
void MainWindow::on_actionCut_triggered()
{
    if (activeMdiChild())
        activeMdiChild()->cut();
}

// 复制菜单
void MainWindow::on_actionCopy_triggered()
{
    if (activeMdiChild())
        activeMdiChild()->copy();
}

// 粘贴菜单
void MainWindow::on_actionPaste_triggered()
{
    if (activeMdiChild())
        activeMdiChild()->paste();
}

// 关闭菜单
void MainWindow::on_actionClose_triggered() { ui->mdiArea->closeActiveSubWindow(); }

// 关闭所有窗口菜单
void MainWindow::on_actionCloseAll_triggered() { ui->mdiArea->closeAllSubWindows(); }

// 平铺菜单
void MainWindow::on_actionTile_triggered() { ui->mdiArea->tileSubWindows(); }

// 层叠菜单
void MainWindow::on_actionCascade_triggered() { ui->mdiArea->cascadeSubWindows(); }

// 下一个菜单
void MainWindow::on_actionNext_triggered() { ui->mdiArea->activateNextSubWindow(); }

// 前一个菜单
void MainWindow::on_actionPrevious_triggered() { ui->mdiArea->activatePreviousSubWindow(); }

// 关于菜单
void MainWindow::on_actionAbout_triggered() { QMessageBox::about(this, tr("关于本软件"), tr("开发者：UestcXiye")); }

// 关于 Qt 菜单
void MainWindow::on_actionAboutQt_triggered()
{
    // 这里的 qApp 是 QApplication 对象的全局指针
    qApp->aboutQt();
    // 这行代码相当于 QApplication::aboutQt();
}

// 更新菜单
void MainWindow::updateMenus()
{
    // 根据是否有活动窗口来设置各个动作是否可用
    bool hasMdiChild = (activeMdiChild() != 0);
    ui->actionSave->setEnabled(hasMdiChild);
    ui->actionSaveAs->setEnabled(hasMdiChild);
    ui->actionPaste->setEnabled(hasMdiChild);
    ui->actionClose->setEnabled(hasMdiChild);
    ui->actionCloseAll->setEnabled(hasMdiChild);
    ui->actionTile->setEnabled(hasMdiChild);
    ui->actionCascade->setEnabled(hasMdiChild);
    ui->actionNext->setEnabled(hasMdiChild);
    ui->actionPrevious->setEnabled(hasMdiChild);
    //设置间隔器是否显示
    actionSeparator->setVisible(hasMdiChild);
    // 有活动窗口且有被选择的文本，剪切复制才可用
    bool hasSelection = (activeMdiChild() && activeMdiChild()->textCursor().hasSelection());
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
    // 有活动窗口且文档有撤销操作时，撤销动作可用
    ui->actionUndo->setEnabled(activeMdiChild() && activeMdiChild()->document()->isUndoAvailable());
    // 有活动窗口且文档有恢复操作时，恢复动作可用
    ui->actionRedo->setEnabled(activeMdiChild() && activeMdiChild()->document()->isRedoAvailable());
}

// 创建子窗口部件
MdiChild* MainWindow::createMdiChild()
{
    // 创建 MdiChild 部件
    MdiChild* child = new MdiChild;
    //向多文档区域添加子窗口，child 为中心部件
    ui->mdiArea->addSubWindow(child);
    // 根据 QTextEdit 类的是否可以复制信号设置剪切复制动作是否可用
    connect(child, SIGNAL(copyAvailable(bool)), ui->actionCut, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)), ui->actionCopy, SLOT(setEnabled(bool)));
    // 根据 QTextDocument 类的是否可以撤销恢复信号设置撤销恢复动作是否可用
    connect(child->document(), SIGNAL(undoAvailable(bool)), ui->actionUndo, SLOT(setEnabled(bool)));
    connect(child->document(), SIGNAL(redoAvailable(bool)), ui->actionRedo, SLOT(setEnabled(bool)));
    // 每当编辑器中的光标位置改变，就重新显示行号和列号
    connect(child, SIGNAL(cursorPositionChanged()), this, SLOT(showTextRowAndCol()));
    return child;
}

// 设置活动子窗口
void MainWindow::setActiveSubWindow(QWidget* window)
{
    // 如果传递了窗口部件，则将其设置为活动窗口
    if (!window)
        return;
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

// 更新窗口菜单
void MainWindow::updateWindowMenu()
{
    // 先清空菜单，然后再添加各个菜单动作
    ui->menuW->clear();
    ui->menuW->addAction(ui->actionClose);     // 关闭
    ui->menuW->addAction(ui->actionCloseAll);  // 关闭所有窗口
    ui->menuW->addSeparator();                 // 分隔符
    ui->menuW->addAction(ui->actionTile);      // 平铺
    ui->menuW->addAction(ui->actionCascade);   // 层叠
    ui->menuW->addSeparator();                 // 分隔符
    ui->menuW->addAction(ui->actionNext);      // 下一个
    ui->menuW->addAction(ui->actionPrevious);  // 前一个
    ui->menuW->addAction(actionSeparator);
    // 如果有活动窗口，则显示间隔器
    QList<QMdiSubWindow*> windows = ui->mdiArea->subWindowList();
    actionSeparator->setVisible(!windows.isEmpty());
    // 遍历各个子窗口
    for (int i = 0; i < windows.size(); i++)
    {
        MdiChild* child = qobject_cast<MdiChild*>(windows.at(i)->widget());
        QString text;
        // 如果窗口数小于 9，则设置编号为快捷键
        if (i < 9)
        {
            text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }
        else
        {
            text = tr("%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }
        // 添加动作到菜单
        QAction* action = ui->menuW->addAction(text);
        // 设置动作可以选择
        action->setCheckable(true);
        // 设置当前活动窗口动作为选中状态
        action->setChecked(child == activeMdiChild());
        // 关联动作的触发信号到信号映射器的 map() 槽函数上，这个函数会发射 mapped() 信号
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        // 将动作与相应的窗口部件进行映射，在发射 mapped() 信号时就会以这个窗口部件为参数
        windowMapper->setMapping(action, windows.at(i));
    }
}

// 显示文本的行号和列号
void MainWindow::showTextRowAndCol()
{
    // 如果有活动窗口，则显示其中光标所在的位置
    if (activeMdiChild())
    {
        // 因为获取的行号和列号都是从 0 开始的，所以我们这里进行了加 1
        int rowNum = activeMdiChild()->textCursor().blockNumber() + 1;
        int colNum = activeMdiChild()->textCursor().columnNumber() + 1;
        ui->statusBar->showMessage(tr("%1行 %2列").arg(rowNum).arg(colNum), 2000);
    }
}
