#ifndef TOOLS_H
#define TOOLS_H

#include <map>
#include <QUrl>
#include <QLabel>
#include <QDebug>
#include <QImage>
#include <QString>
#include <QWidget>
#include <optional>
#include <QLineEdit>
#include <type_traits>
#include <initializer_list>

struct LabelState{
    LabelState();

    /*
     * Default status = DISABLE
     * VisiableStatus will be changed after every clicked!
    */
    enum VisiableStatus{
        DISABLED,
        ENABLED
    }visiable;

    enum class HoverStatus: uint8_t{
        DISABLED,
        ENABLED
    }hover;
};


template<typename Widget, class = void>
struct has_settext_function: std::false_type
{};

template<typename Widget>
struct has_settext_function<Widget,
                            std::void_t<decltype(std::declval<std::decay_t<Widget>>().setText(QString()))>>
    : std::true_type
{};

struct Tools
{
    static void refreshQssStyle(QWidget *widget);

    template<typename Widget,
        typename std::enable_if<has_settext_function<std::remove_pointer_t<Widget>>::value, int>::type = 0
    >
    static void setWidgetAttribute(Widget widget, const QString &message, bool status){
        widget->setText(message);
        widget->setProperty("state", (status ? "correct" : "incorrect"));
        refreshQssStyle(widget);
    }

    /*handling url info*/
    static void readConfigrationFile(QString file_name = QT_DEMO_HOME"config.ini");
    static QUrl getTargetUrl(QString param = "");   //for public access
    static QString url_info;                //store url info
    static bool url_init_flag;              //url is init or not?

    /*check validation*/
    static bool checkUsername(QLineEdit *edit, QLabel *label);
    static bool checkEmail(QLineEdit *edit, QLabel *label);
    static bool checkPassword(QLineEdit *edit, QLabel *label);
    static bool checkSimilarity(QLineEdit *edit_pass, QLineEdit *edit_confirm, QLabel *label);
    static bool checkCaptcha(QLineEdit *edit, QLabel *label);

    /*load image*/
    static std::map<QString, QImage> s_images;

    /*all images files should be inside "/res" dir*/
    static void loadImgResources(std::initializer_list<QString> file_list, int width, int height);
    static void setQLableImage(QLabel *label, const QString &target);
    static std::optional<QImage> loadImages(const QString &path, int width, int height);
};

#endif // TOOLS_H
