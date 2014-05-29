#ifndef HGEMAINWINDOW_HPP
#define HGEMAINWINDOW_HPP
#include <QtWidgets/QWidget>
namespace Ui
{
	class MainWindow;
}
class MainWindow : public QWidget
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
private:
	Ui::MainWindow *ui;
	qint16 *imagH;
	quint64 textureSize;
	double vd, hd;
	bool imgLoaded;
private slots:
	void onOpenClicked();
	void onSaveClicked();
	void onSaveHHClicked();
};
#endif
