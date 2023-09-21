#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QtSql>
#include<QSqlQuery>
#include<QSqlError>
#include<QtNetwork>
#include<QDate>
#include<QDateTime>
#include<QTime>
#include<QDebug>

#include<QMessageBox>

//Bibliotecas para opencv
#include<opencv2/core/core.hpp>
#include<opencv2/ml/ml.hpp>
#include<opencv/cv.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/video/background_segm.hpp>
#include<opencv2/videoio.hpp>
#include<opencv2/imgcodecs.hpp>

#include<QTimer>

#include "mat2qimage.h"

using namespace cv;
//VideoCapture camara(2);  //Enlazar mi camara USB externa
VideoCapture camara(0);  //Camara de la laptop.
//VideoCapture camara(http://Direccion:8080/video);
Mat imagen;



void MainWindow :: fTimer(){
    //Paso #19.1 Capturar una imagen de la camara.
    camara >> imagen;

    //Paso #19.2 Cambiar el tamaño, de la imagen anterior a 250x150.
    Mat imagenChica;
    cv::resize(imagen,imagenChica, Size(250,150),0,0,0);

    //Paso #19.3 Mostrar la imagen en la carpeta label_6.

    //Paso #19.3.1 Cambiar la imagen MAT a una imegen QImage.
    QImage imagenQT = Mat2QImage(imagenChica);

    //PAso #19.3.2 Cambiar la Imagen de QT a mapa de Pxeles QPixMap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso #19.3.3 Limpiar la etiqueta 6.
    ui->label_6->clear();

    //Paso #19.3.4 Mostrar el mapa de pixeles en la etiqueta label_6.
    ui->label_6->setPixmap(mapaPixeles);
}

QSqlDatabase baseDeDatos = QSqlDatabase::addDatabase("QMYSQL");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Paso #16.1
    //configuración del cronometro 1, a 30 ms.
    QTimer *cronometro = new QTimer(this);
    connect(cronometro, SIGNAL(timeout()),SLOT(fTimer()));
    cronometro->start(30);

    //configuración #1.configurarar la base de datos.
    baseDeDatos.setHostName("localhost");
    baseDeDatos.setPort(3306);
    baseDeDatos.setDatabaseName("2021B2");
    baseDeDatos.setUserName("admin51");
    baseDeDatos.setPassword("hola1234");



}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_lineEdit_2_returnPressed()
{
    //Paso #1. Guardar en una variable el texto del LineEdit_2
    QString contrasena1 = ui->lineEdit_2->text();
    //Paso #2. Declarar una variable booleana que le inidique al programa si la contraseña coincide con alguna almacenada
    bool contrasenaValida = false;
    //si contrasenaValida = verdadero significa que la contraseña ingresada existe en la base de datos.
    //Paso #3. Abrir la base de datos.
    if(baseDeDatos.open()){
        qDebug() << "OK1";

        //Paso # 4. Crear un comando de texto para LEER la información de la tabla configuracion
        QString comandoTexto = "SELECT * FROM configuracion ORDER BY id DESC";
        //Paso # 5 convertir el comando anterior a un comando de mySQL
        QSqlQuery comando;
        comando.prepare(comandoTexto);

        //Paso # 6. Ejecutar comando anterior.
        if(comando.exec()){
            //se logro una conexion con la tabla configuración
            qDebug() << "OK2";
            //Paso # 7. Leer iterativamente cada fila de la tabla configuración
            while(comando.next()){
                //Paso # 8. Leer la columna correspondiente a la contraseña
                //, de cada fila.
                // comando.value(numeroColumna).conversion
                //int identificador = comando.value(0).toInt();
                QString contrasena2 = comando.value(1).toString();
                //Paso # 9. comparar las contraseñas.
                if(contrasena1==contrasena2){
                    contrasenaValida = true;
                }
            }
        }
        else{
            qDebug() << "Error 2- No se logro una conexión con la tabla";
            //Razones
            qDebug() << "Razon #1. El comando de mysql, del paso 4, esta incorrecto";
            qDebug() << "Razon #2. Escribimos mal el nombre de la tabla";
        }

    }
    else{
        qDebug() << "Error 1 - La base de datos no se pudo abrir";
        //Razones
        qDebug() << "Razon #1. Puede que la contraseña de configuración este erronea";
        qDebug() << "Razon #2. Puede que el usuario de configuración este erronea";
        qDebug() << "Razon #3. Puede ser que exista el usuario y la contraseña sea valida, pero que dicho usuario no pueda usar la base de datos";
        qDebug() << "Razon #4. Puede que el nombre de la base de datos este erroneo";
        qDebug() << "Razon #5. se configuro de forma erronea el hostname";

    }
    //Paso #10  si la contraseña es valida, habilitamos a los elementos correspondientes
    if(contrasenaValida){
        ui->lineEdit_3->setEnabled(true);
        ui->lineEdit_4->setEnabled(true);
        ui->lineEdit_5->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
    }
    else{
        ui->lineEdit_3->setEnabled(false);
        ui->lineEdit_4->setEnabled(false);
        ui->lineEdit_5->setEnabled(false);
        ui->pushButton_2->setEnabled(false);

    }
 }


void MainWindow::on_pushButton_2_clicked()
{
    bool codigoExiste = false;
    //Paso 11.1 guardar la información en variables
    QString nombre = ui->lineEdit_3->text();
    QString codigo = ui->lineEdit_4->text();
    QString seccion = ui->lineEdit_5->text();

    //Paso #11.2 Guardar una imagen de camara web en un archivo
    QString nombreImagen = codigo+".jpg";
    cv:: imwrite(nombreImagen.toUtf8().constData(),imagen);

    //Paso #13 Abrir la base de datos.
    if(baseDeDatos.open()){
        qDebug() << "OK1, abrir base de datos";
        //Paso # 11.4 Crear el comando de mysql para insertar un dato nuevo.
        //Para esto, primero, tenemos que validar, que el codigo a insertar,
        //no exista previamente en la base de datos.
        QString comando1 = "SELECT * FROM listaPrincipal WHERE codigo= '"+codigo+"'";

        QSqlQuery comando2;
        comando2.prepare(comando1);

        if(comando2.exec()){
            qDebug() << "El comando del paso # 11.4 se ejecuto correctamente";
            //Verificer si existe un codigo que sea igual al ingresado previamente
            while (comando2.next()) {
                QString codigoEncontrado = comando2.value(2).toString();
                if(codigoEncontrado.isEmpty()){
                  codigoExiste = false;
                }
                else{
                    codigoExiste = true;
                }
            }
        }
        else{
            qDebug() << "Error, el comando del paso #11.4 NO esta bien";
        }
    }
    else{
        qDebug()<< "Error al abrir la base de datos";
    }

    //Paso #11.5 verificamos si el codigo existe
    if(codigoExiste){
        qDebug() << "El codigo ya existe, ingrese uno nuevo";
        //Paso #13.
        QMessageBox::critical (this,tr("Error"),tr("El codigo ya existe, intente de nuevo"));
    }
    else{
        qDebug() << "El codigo es nuevo, por lo tanto puede insertar una nueva fila a la ltabla lista Principal";
        //Paso #11.6 Insertar la información NUEVA en la tabla lista principal
        QString comando3 = "INSERT INTO listaPrincipal(nombre, codigo, foto, seccion) VALUES(?,?,?,?)";
        QSqlQuery comando4;
        comando4.prepare(comando3);
        comando4.addBindValue(nombre);
        comando4.addBindValue(codigo);
        comando4.addBindValue(nombreImagen);
        comando4.addBindValue(seccion);
        //Paso # 11.7 Insertar el valor en la tabla correspondiente.
        if(comando4.exec()){
            qDebug() << "La información se agrego correctamente";
        }
        else{
            qDebug() << "El comando no se agrego a la tabla y hay un error en el paso 11.6";
        }
        //Paso #11.8 Limpiar y deshabilitar a los elementos de entrada
        ui->lineEdit_2->clear();
        ui->lineEdit_3->clear();
        ui->lineEdit_4->clear();
        ui->lineEdit_5->clear();

        ui->lineEdit_3->setEnabled(false);
        ui->lineEdit_4->setEnabled(false);
        ui->lineEdit_5->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    }
}


void MainWindow::on_pushButton_clicked()
{
    //obtener la fecha.
    //formato UNIX
    long int fechaUnix = QDateTime::currentSecsSinceEpoch();

    //Formato String
    QDateTime tiempo= QDateTime::currentDateTime();
    QString fechaString = tiempo.toString();

    ui->lineEdit_6->setText(fechaString);
    //Paso #20. Guardar en una variable el texto del LineEdit_2
    QString codigo = ui->lineEdit->text();


    //Paso #21. Declarar una variable booleana que le inidique al programa si la contraseña coincide con alguna almacenada
    bool codigoValido = false;
    QString nombreEncontrado;
    //si codigoValido = verdadero significa que la contraseña ingresada existe en la base de datos.
    //Paso #22. Abrir la base de datos.
    if(baseDeDatos.open()){
        qDebug() << "OK3, abri base de datos";
        //Paso 22.1 Crear un comando para insertar un nuevo dato en la tabla asistencia,
        //Para eso tenemos que validar que el codio ingresado existé dado de alta en la tabla listaPrincipal
        //Crear un comando de MYSQL para validar que el codigo exista en la tabla Lista Principal
        QString comando5 = "SELECT * FROM listaPrincipal WHERE codigo= '"+codigo+"'";

        QSqlQuery comando6;

        comando6.prepare(comando5);
        if(comando6.exec()){
            qDebug() << "El comando se ejecuto correctamente";
            //Verificar si existe el codigo que sea igual al ingresado previamente
            while(comando6.next()){
                QString codigoEncontrado = comando6.value(2).toString();
                nombreEncontrado = comando6.value(1).toString();
                if(codigoEncontrado.isEmpty()){
                    codigoValido = false;
                }
                else
                    codigoValido = true;
            }
        }
        else{
            qDebug() << "Error el comando del paso 22.1 NO esta bien ";
        }

    }
    else{
        qDebug() << "Error, no se pudo abrir la base de datos";
    }
    //Paso #22.2 verificar si el codigo existe
        if(codigoValido){
            qDebug()<<"El codigo existe";
            //Paso #22.3 Insertar información en la tabla Asistencia
            QString comando7 = "INSERT INTO asistencia(nombre, codigo, fecha) VALUES(?,?,?)";
            QSqlQuery comando8;
            comando8.prepare(comando7);
            comando8.addBindValue(nombreEncontrado);
            comando8.addBindValue(codigo);
            comando8.addBindValue(QString::number(fechaUnix));

            if(comando8.exec()){
                qDebug()<<"La fila se insertó";
                QMessageBox::critical (this,tr("Hecho"),tr("Asistencia Registrada"));
            }
            else{
                qDebug()<<"Error";
            }
        }
        else{
            qDebug()<<"El codigo NO existe, favor de darlo de alta";
            QMessageBox::critical (this,tr("Error"),tr("El codigo NO existe, Favor de darlo de alta"));
        }

}
