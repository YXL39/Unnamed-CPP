#include "C++bianjiqi.h"
using namespace std;

// 安全参数验证
bool isSafeCompilerArg(const QString& arg) {
    static QRegularExpression dangerousChars(R"([;&|<>`$(){}'"\n\r\t])");
    static QStringList allowedOptions = {
        "-I", "-L", "-l", "-std=", "-O", "-g", "-Wall", "-Wextra"
    };
    
    // 检查危险字符
    if(arg.contains(dangerousChars)) return false;
    
    // 检查是否允许的编译选项
    if(arg.startsWith("-")) {
        for(const auto& opt : allowedOptions) {
            if(arg.startsWith(opt)) return true;
        }
        return false;
    }
    
    // 普通文件路径
    return QFile::exists(arg) || 
           arg.endsWith(".cpp") || 
           arg.endsWith(".h");
}
int main(int argc,char* argv[]){
	QApplication app(argc, argv);
	QMainWindow window;
	window.showMaximized();
	window.setWindowTitle("C++编辑器");
	QMenuBar* menuBar = window.menuBar();
	QMenu* fileMenu = menuBar->addMenu(("文件[&F]"));
	QMenu* run = menuBar->addMenu(("运行[&C]"));
	
	// 添加工具栏和编译器选择
	QToolBar* toolBar = window.addToolBar("Main Toolbar");
	QLabel* compilerLabel = new QLabel("");
	QComboBox* compilerComboBox = new QComboBox();
	compilerComboBox->addItem("TDM-GCC 4.9.2 64-bit Release");
	toolBar->addWidget(compilerLabel);
	toolBar->addWidget(compilerComboBox);
	
	QAction* newAction = new QAction(("新建[&N]"), &window);
	QAction* openAction = new QAction(("打开[&O]"), &window);
	QAction* saveAction = new QAction(("保存[&S]"), &window);
	QAction* exitAction = new QAction(("退出[&X]"), &window);
	QAction* compileAction = new QAction(("编译[&R]"), &window);
	fileMenu->addAction(newAction);
	fileMenu->addAction(openAction);
	fileMenu->addAction(saveAction);
	fileMenu->addSeparator(); 
	fileMenu->addAction(exitAction);
	run->addAction(compileAction);
	QLabel* label = new QLabel("欢迎回来，未登录用户", &window);
	QFont font("Arial", 48);
	label->setFont(font);
	label->move(175, 50);
	label->resize(650, 100);
	QTextEdit* textEdit = new QTextEdit(&window);
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(textEdit);
	QWidget* centralWidget = new QWidget(&window);
	centralWidget->setLayout(layout);
	textEdit->hide();
	window.setCentralWidget(centralWidget);
	//A区 定义区
	QObject::connect(newAction, &QAction::triggered, [&]() {
		label->hide();
		QString fileName = QFileDialog::getSaveFileName(&window, "保存", "", "CPP Files (*.cpp)");
		if (!fileName.isEmpty()) {
			textEdit->clear();
			textEdit->show();
			QFile file(fileName);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				QTextStream out(&file);
				out << textEdit->toPlainText();
				file.close();
			}
			else {
				QMessageBox::warning(&window, "错误", "无法创建文件: " + file.errorString());
			}
		}
		});
	QObject::connect(saveAction, &QAction::triggered, [&]() {
		label->hide();
		QString fileName = QFileDialog::getSaveFileName(&window, "保存", "", "CPP Files (*.cpp)");
		if (!fileName.isEmpty()) {
			textEdit->show();
			QFile file(fileName);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				QTextStream out(&file);
				out << textEdit->toPlainText();
				file.close();
			}
			else {
				QMessageBox::warning(&window, "错误", "无法保存文件: " + file.errorString());
			}
		}
		});
	QObject::connect(openAction, &QAction::triggered, [&]() {
		QString fileName = QFileDialog::getOpenFileName(&window, "打开", "", "CPP Files (*.cpp)");
		if (!fileName.isEmpty()) {
			QFile file(fileName);
			if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream in(&file);
				textEdit->setText(in.readAll());
				textEdit->show();
				file.close();
			}
			else {
				QMessageBox::warning(&window, "错误", "无法打开文件: " + file.errorString());
			}
		}
		});

	// 编译器选择变更处理
	QObject::connect(compilerComboBox, &QComboBox::currentTextChanged, [&](const QString &text) {
		QMessageBox::information(&window, "编译器已更改", 
			"已选择编译器: " + text);
	});

	QObject::connect(exitAction, &QAction::triggered, [&]() {
		int ret = QMessageBox::question(&window, "退出", "您确定要退出吗？",
			QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::Yes) {
			app.quit();
		}
		});
	QObject::connect(compileAction, &QAction::triggered, [&]() {
		// 该地区由AI辅助编写
		// 首先选择编译器路径
		QString compilerPath = QFileDialog::getOpenFileName(&window, "选择编译器", "", "Executable Files (*.exe)");
		if(compilerPath.isEmpty()) return;
		
		// 然后选择源代码文件
		QString fileName = QFileDialog::getSaveFileName(&window, "保存并编译", "", "CPP Files (*.cpp)");
		if (!fileName.isEmpty()) {
			// 保存当前编辑内容到临时文件
			QString tempFileName = fileName + ".tmp";
			QFile tempFile(tempFileName);
			if (tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
				QTextStream out(&tempFile);
				out << textEdit->toPlainText();
				tempFile.close();
				
				// 验证临时文件内容是否与编辑器一致
				if(tempFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QString savedContent = tempFile.readAll();
					tempFile.close();
					if(savedContent != textEdit->toPlainText()) {
						QMessageBox::warning(&window, "错误", "保存内容验证失败");
						return;
					}
				}
				
				// 确认无误后重命名为正式文件
				QFile::remove(fileName);
				if(!QFile::rename(tempFileName, fileName)) {
					QMessageBox::warning(&window, "错误", "无法保存文件");
					return;
				}
				
				// 执行编译
				QProcess process;
				QString program = compilerPath;
				QStringList arguments;
				
				// 验证编译器是否存在
				if(!QFile::exists(compilerPath)) {
					QMessageBox::warning(&window, "错误", "指定的编译器路径不存在");
					return;
				}
				// 确保输出路径与源文件不同
				QString baseName = QFileInfo(fileName).fileName();
				if(baseName.endsWith(".cpp")) {
					baseName.chop(4); // 移除.cpp
				}
				QString outputName = QFileInfo(fileName).absoluteDir().absoluteFilePath(baseName + ".exe");
				
				// 二次确认输出路径不会覆盖源文件
				if(QFileInfo(outputName).canonicalFilePath() == QFileInfo(fileName).canonicalFilePath()) {
					QMessageBox::warning(&window, "错误", "输出路径与源文件冲突");
					return;
				}
				
				arguments << fileName << "-o" << outputName;
				
				// 确保输出目录可写
				QString outputDir = QFileInfo(fileName).absolutePath();
				QFile testFile(outputDir + "/test_write.tmp");
				if(!testFile.open(QIODevice::WriteOnly)) {
					QMessageBox::warning(&window, "错误", "输出目录不可写，请检查权限");
					return;
				}
				testFile.close();
				testFile.remove();
				
				process.start(program, arguments);
				process.waitForFinished();
				
				if(process.exitCode() == 0) {
					if(QFile::exists(outputName)) {
						QMessageBox::information(&window, "编译成功", 
							QString("程序编译成功！\n生成的可执行文件:\n%1\n\n点击确定运行程序").arg(outputName));
						
						// 运行生成的可执行文件
						QProcess::startDetached(outputName);
					} else {
						QMessageBox::warning(&window, "错误", 
							"可执行文件未生成在预期位置: " + outputName);
					}
				} else {
					QString error = process.readAllStandardError();
					QMessageBox::warning(&window, "编译错误", error);
					QMessageBox::warning(&window, "编译错误", error);
				}
			}
		}
	});
	//B区 事件区
	window.show();
	return app.exec();
}