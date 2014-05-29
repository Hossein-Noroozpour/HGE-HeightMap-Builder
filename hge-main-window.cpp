#include "hge-main-window.hpp"
#include "ui_hge-main-window.h"
#include <QtWidgets/QMessageBox>
#include <QtCore/QFile>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDebug>
#include <QtCore/QtMath>
#define NASATEXTUREHEIGHT 43200
#define NASATEXTUREWIDTH 86400
MainWindow::MainWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MainWindow),
	imgLoaded(false)
{
	ui->setupUi(this);
	setWindowTitle("Hulixerian Game Engine: Height Map Creator");
	ui->vdLE->setValidator(new QDoubleValidator(-90.0, 90.0, 10));
	ui->hdLE->setValidator(new QDoubleValidator(-180.0, 180.0, 10));
	connect(ui->oPB, SIGNAL(clicked()), this, SLOT(onOpenClicked()));
	connect(ui->sPB, SIGNAL(clicked()), this, SLOT(onSaveClicked()));
	connect(ui->shhmPB, SIGNAL(clicked()), this, SLOT(onSaveHHClicked()));
}
MainWindow::~MainWindow()
{
	delete ui;
	if(imgLoaded)
	{
		delete [] imagH;
	}
}
void MainWindow::onOpenClicked()
{
	bool ok = false;
	hd = ui->hdLE->text().toDouble(&ok);
	if(!ok || -180.0 > hd || 180.0 < hd)
	{
		QMessageBox msg(this);
		msg.information(this, "Invalid horizontal degree number", "Horizontal number must be in range (-180.0 , 180.0).");
		return;
	}
	ok = false;
	vd = ui->vdLE->text().toDouble(&ok);
	if(!ok || -90.0 > vd || 90.0 < vd)
	{
		QMessageBox msg(this);
		msg.information(this, "Invalid vertical degree number", "Vertical number must be in range (-90.0 , 90.0).");
		return;
	}
	quint64 vI = (quint64)(((90.0  - vd) / 180.0) * NASATEXTUREHEIGHT);
	quint64 hI = (quint64)(((180.0 + hd) / 360.0) * NASATEXTUREWIDTH );
	unsigned int tS = ui->ts512RB->isChecked()?512:ui->ts1024RB->isChecked()?1024:ui->ts2048RB->isChecked()?2048:4096;
	textureSize = tS * tS * 2;
	unsigned int tH = (tS + vI);
	unsigned int tW = (tS + hI);
	if(tH > NASATEXTUREHEIGHT || tW > NASATEXTUREWIDTH)
	{
		QMessageBox msg(this);
		msg.information(this, "Invalid range of degree numbers",
						"Degrees and terrain size must put terrain squar in a uniform page.");
		return;
	}
	QFileDialog fc(this, "Select NASA Height-Map file", QString(), "bin");
	QFile nasaHMF(fc.getOpenFileName());
	if(!nasaHMF.open(QIODevice::ReadOnly))
	{
		QMessageBox msg(this);
		msg.information(this, "Error in file opening.",
						"The selected file can not be opened properly.");
		return;
	}
	qint16 *heights = new qint16[tS * tS];
	for(unsigned int h = 0, endI = tS * tS, fLoc = ((vI * NASATEXTUREWIDTH) + hI) * 2, fileR = tS * 2;
		h < endI; h += tS, fLoc += (NASATEXTUREWIDTH * 2))
	{
		nasaHMF.seek(fLoc);
		if(fileR != nasaHMF.read((char *)(&(heights[h])), fileR))
		{
			QMessageBox msg(this);
			msg.information(this, "Error in file reading.",
							"The selected file can not be read properly.");
			return;
		}
	}
	if(imgLoaded)
	{
		delete [] imagH;
	}
	imgLoaded = true;
	imagH = heights;
	QImage img((uchar *)imagH, tS, tS, QImage::Format_RGB16);
	ui->hmL->setPixmap(QPixmap::fromImage(img));
}
void MainWindow::onSaveClicked()
{
	if(!imgLoaded)
	{
		QMessageBox msg(this);
		msg.information(this, "Error in saving.",
						"You do not specify an image yet.");
		return;
	}
	QFileDialog fc(this, "Save height map file");
	ui->hmL->pixmap()->toImage().save(fc.getSaveFileName(this, "Save height-map", "", "jpg"),"jpg", 100);
}
void MainWindow::onSaveHHClicked()
{
	if(!imgLoaded)
	{
		QMessageBox msg(this);
		msg.information(this, "Error in saving.",
						"You do not specify an image yet.");
		return;
	}
	QFileDialog fc(this, "Save height map file", "", "hge");
	QFile hhmF(fc.getSaveFileName());
	if(!hhmF.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		QMessageBox msg(this);
		msg.information(this, "Error in file opening.",
						"The selected file can not be opened properly.");
		return;
	}
	quint8 endianType;
	{
		int checknum = 1;
		if(*((char *)(&checknum)) == 1)
		{ /// Little-Endian
			endianType = 0;/// This means: Little endian
		}
		else
		{ /// Big-Endian
			endianType = 1;/// This means: Big endian
		}
		hhmF.write((char *)&endianType, sizeof(quint8));
	}
	quint64 objectType = 0;/// This means: This object is terrain.
	hhmF.write((char *)&objectType, sizeof(quint64));
	quint32 terrainAspect = (quint32)qSqrt(qreal(textureSize/2));
	hhmF.write((char *)&terrainAspect, sizeof(quint32));
	hhmF.write((char *)&hd, sizeof(double));
	hhmF.write((char *)&vd, sizeof(double));
	if(endianType == 0)
	{
		unsigned char *imagBytes = (unsigned char *)imagH;
		for(unsigned int i = 0; i < terrainAspect * terrainAspect * 2; i += 2)
		{
			unsigned char tmpchar = imagBytes[i];
			imagBytes[i] = imagBytes[i + 1];
			imagBytes[i + 1] = tmpchar;
		}
	}
	if(textureSize != hhmF.write((char *)imagH, textureSize))
	{
		QMessageBox msg(this);
		msg.information(this, "Error in file writing.",
						"The selected file can not be written properly.");
		return;
	}
}
