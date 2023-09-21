#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "telegrambot.h"
#include "mat2qimage.h"
#include<QDebug>
#include <QDirIterator>
#include<QDebug>
#include<QFile>
#include <QTextStream>
#include<QtNetwork/QtNetwork>
#include<QTimer>
#include<opencv2/core/core.hpp>
#include<opencv2/ml/ml.hpp>
#include<opencv/cv.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/video/background_segm.hpp>
#include<opencv2/videoio.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/objdetect.hpp>
#include <QSqlDatabase>
#include <QtSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QtNetwork>
#include <QDate>
#include <QDateTime>
#include <QTime>


TelegramBot interfacesbot("1908824198:AAGoXf-YDGNm8-qCmTtTX4uzqtV68edVG_A");
using namespace cv;

VideoCapture camara(0);
Mat imagen;

QSqlDatabase baseDeDatos = QSqlDatabase::addDatabase("QMYSQL");
// send message (Format: Normal)
TelegramBotMessage msgSent;


//Paso # 8.0 Declarar una variable booleana para que la verificación del usuario registrado, solo se realice en una sola ocasión.
bool usuarioVerificado = false;

//Paso # 8.01 Declarar una variable booleana para saber si registro o no al usuario
bool usuarioRegistrado = false;

//Paso # 8.02 Declarar una variable booleana para saber si registro o no al usuario
bool nombreRegistrado = false;

//Paso # 8.02 Declarar una variable booleana para saber si registro o no al usuario
bool CaraEncontrada = false;

//Paso # 9.2 - Variable publica que enlaza a los archivos XML.
CascadeClassifier deteccionRostros;

//Paso # 9.4
int contadorTiempo = 0;

void MainWindow::Temporizador(){
    TelegramBotMessage msgSent;
    camara >> imagen;

    //Cambiar el tamaño
           Mat imagenChica;
           cv::resize(imagen, imagenChica, Size(650,350),0,0,INTER_LINEAR);


           //Paso # 9.4 Crear una función que se ejecute cada minuto
           contadorTiempo++;
           if(contadorTiempo >= 20*10){
               //Ocurrio 1 segundo
               qDebug() << "Paso aproximadamente 1 minuto de tiempo";
               contadorTiempo = 0;

               //Paso # 9.5 Verificar si la imagen contiene un rostro o no.
               //Paso # 9.5.1 Crear una imagen GRIS a partir de la imagenChica
               Mat imagenGris;
               cvtColor(imagenChica, imagenGris, COLOR_BGR2GRAY);

               //Paso # 9.5.2 Equalizar la imagen anterior.
               equalizeHist(imagenGris,imagenGris);

               //Paso # 9.5.3 Crear una variable que permita guardar las cooordenadas de las caras encontradas
               std::vector<Rect> carasEncontradas;

               //Paso # 9.5.4 Hacer la detección
               deteccionRostros.detectMultiScale(imagenGris, carasEncontradas, 1.1, 2, 0|CASCADE_SCALE_IMAGE,Size(30,30));

               //Paso # 9.5.6 Para cada cara encontrada, dibuja un rectangulo, que le haga saber al usuario, que se encontro una cara.
               for(size_t i = 0; i<carasEncontradas.size();i++) {
                   Point centroElipse(carasEncontradas[i].x+carasEncontradas[i].width/2,carasEncontradas[i].y+carasEncontradas[i].height/2);
                   ellipse(imagenChica, centroElipse, Size(carasEncontradas[i].width/2,carasEncontradas[i].height/2),0,0,360, Scalar(255,0,255),4,8,0);
                   CaraEncontrada = true;
               }
                   //Guardar imagen del Rostro
                    imwrite("../caraEncontrada.jpg",imagenChica);
                    if(CaraEncontrada){
                        CaraEncontrada = false;

                   //comando para enviar foto a los usuarios registrados
                   QString comandoTexto = "SELECT * FROM ejemplo5 ORDER BY id";
                   QSqlQuery comando;
                   comando.prepare(comandoTexto);
                   if(comando.exec()){
                        while(comando.next()){
                            long int identificador = comando.value(1).toInt();
                            QVariant valor = QVariant::fromValue(identificador);

                            interfacesbot.sendMessage(valor, "Gato encontrado",0,TelegramBot::NoFlag, TelegramKeyboardRequest(),&msgSent);
                            interfacesbot.sendPhoto(valor,"../caraEncontrada.jpg","Alerta michi Encontrado");
                        }
                   }
                 }



           }


           //Convertir la imagen Mat a una imagen de QT
           QImage imagenQT = Mat2QImage(imagenChica);

           //Convertir la imagen de QT a un mapa de pixeles de QT
           QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

           //Limpiar el contenido de la etiqueta
           ui->label->clear();

           //Mostrar el mapa de pixeles en la etiqueta
           ui->label->setPixmap(mapaPixeles);

    }

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //9.3 Cargar el archivo XML al clasificador en cascada.
        if(deteccionRostros.load("../HAAR/CaraFrontalGatoE.xml")){
            qDebug() << "El archivo XML se cargo exitosamente";
        }
        else{
            qDebug() << "Error en la carga del archivo";
            qDebug() << "(1) La ubicación dada, no existe";
            qDebug() << "(2) El archivo no existe donde se esta indicando";
        }

        //Configuración # 1. Configurar la base de datos.
           //baseDeDatos.setHostName("https://hetpro-store.com");
           //Configurar el servidor de la base de datos.
           baseDeDatos.setHostName("localhost");

           //Configurar el puerto a donde se conectara a la base de datos.
           baseDeDatos.setPort(3306);

           //Configurar la base de datos a la que nos vamos a conectar (esta ya debe de existir).
           baseDeDatos.setDatabaseName("2021B2");

           //Configurar el usuario que tenga permisos de modificar la base de datos.
           baseDeDatos.setUserName("admin51");

           //Configurar la contraseña del usuario anterior.
           baseDeDatos.setPassword("hola1234");

           if(baseDeDatos.open()){
               qDebug() << "Abri la base de datos";
           }
           else{
               qDebug() << "Error al abrir la base de datos";
           }



        //Crear cronometro
            QTimer *cronometro = new QTimer(this);

            //Configurar
            connect(cronometro, SIGNAL(timeout()), this, SLOT(Temporizador()));

            //Iniciar
            cronometro->start(50);


            QObject::connect(&interfacesbot, &TelegramBot::newMessage, [&interfacesbot](TelegramBotUpdate update) {
                // only handle Messages
                if(update->type != TelegramBotMessageType::Message) return;

                // simplify message access

                TelegramBotMessage& message = *update->message;


                QString mensajeRecibido = message.text;

                //Paso # 8.1 Guardar el identificador del usuario
                QString identificador = QString::number(message.chat.id);

                //Paso # 8.2 Primero verificar que el usuario este dado de alta.
                if(!usuarioVerificado){


                    //Paso # 8.3 Crear el comando de mysql que permita verificar la fila anterior en la tabla de usuarios.
                    QString comandoTexto = "SELECT * FROM ejemplo5 WHERE chatID = '"+identificador+"'";

                    //Paso # 8.4 Enlazar el comando anterior a un comando de sql
                    QSqlQuery comando;
                    comando.prepare(comandoTexto);

                    //Paso # 8.5 Ejecutar el comando anterior.
                    if(comando.exec()){
                        usuarioVerificado = true;
                        qDebug() << "Ok1";
                        //Paso # 8.6 Leer cada fila que coincida con el campo buscado
                        while (comando.next()) {
                            QString identificadorEncontrado = comando.value(1).toString();
                            if(identificadorEncontrado.isEmpty()){
                                usuarioRegistrado = false;
                            }
                            else{
                                usuarioRegistrado = true;
                                qDebug() << "El usuario YA EXISTE!";
                                interfacesbot.sendMessage(message.chat.id,
                                                "Ya estas REGISTRADO!",
                                                0,
                                                TelegramBot::NoFlag,
                                                TelegramKeyboardRequest(),
                                                &msgSent);

                            }

                        }
                    }
                    else{
                        qDebug() << "Error1";

                    }
                    if(usuarioRegistrado == false){
                        qDebug() << "El usuario NO existe, hay que registrarlo";

                    }

                }

                //Paso 8.7 Verificar que el mensaje recibido contenga la palabra nombre:
                //nombre:RUBEN ESTRADA
                int caracteresEncontrados = mensajeRecibido.indexOf("nombre:");

                //Paso 8.8 Si el mensaje recibido, corresponde al nombre, se guarda en la base de datos.

                if(caracteresEncontrados >= 0 && nombreRegistrado == false){
                    QString comandoTexto2 = "INSERT INTO ejemplo5(chatID, nombre, fecha) VALUES(?,?,?)";
                    long int fechaUnix = QDateTime::currentSecsSinceEpoch();
                    QString fechaUnixTexto = QString::number(fechaUnix);

                    QSqlQuery comando2;
                    comando2.prepare(comandoTexto2);
                    comando2.addBindValue(identificador);
                    comando2.addBindValue(mensajeRecibido);
                    comando2.addBindValue(fechaUnixTexto);

                    if(comando2.exec()){
                        qDebug() << "OK2 El usuario se registro correctamente";
                        nombreRegistrado = true;
                        usuarioRegistrado = true;
                        interfacesbot.sendMessage(message.chat.id,
                                        "Tu REGISTRO se guardo correctamente",
                                        0,
                                        TelegramBot::NoFlag,
                                        TelegramKeyboardRequest(),
                                        &msgSent);

                    }
                    else{
                        qDebug() << "Error en el comando 2";
                    }
                }

                if(mensajeRecibido == "foto"){
                    qDebug() << "Identificador del usuario: " << message.chat.id;
                    interfacesbot.sendMessage(message.chat.id,
                                    "En un momento te enviare la foto",
                                    0,
                                    TelegramBot::NoFlag,
                                    TelegramKeyboardRequest(),
                                    &msgSent);

                    camara >> imagen;
                    cv::imwrite("../imagen.jpg", imagen);
                    interfacesbot.sendPhoto(message.chat.id,"../imagen.jpg","Te paso tu foto...");

                }



                //Si el usuario no existe, hay que pedirle que se registre.
                if(usuarioRegistrado == false){
                     if(nombreRegistrado == false){
                        interfacesbot.sendMessage(message.chat.id,
                                        "Hola, favor de registrarte, escribe tu nombre, como: nombre:TUNombre",
                                        0,
                                        TelegramBot::NoFlag,
                                        TelegramKeyboardRequest(),
                                        &msgSent);
                    }

                }





            });
            interfacesbot.startMessagePulling();

    }

MainWindow::~MainWindow()
{
    delete ui;
}
