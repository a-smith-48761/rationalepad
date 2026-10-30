#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QLabel>
#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("Qt + SQLite smoke test");
    auto *central = new QWidget(&window);
    auto *layout = new QVBoxLayout(central);
    auto *label = new QLabel(central);
    label->setWordWrap(true);
    layout->addWidget(label);
    window.setCentralWidget(central);

    QString storage = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(storage);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir(storage).filePath("example.sqlite"));
    if (!db.open()) {
        label->setText("SQLite open failed: " + db.lastError().text() +
                       "\nAvailable SQL drivers: " + QSqlDatabase::drivers().join(", "));
    } else {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE IF NOT EXISTS launches (id INTEGER PRIMARY KEY, at TEXT DEFAULT CURRENT_TIMESTAMP)")) {
            label->setText("SQLite create failed: " + query.lastError().text());
        } else if (!query.exec("INSERT INTO launches DEFAULT VALUES")) {
            label->setText("SQLite insert failed: " + query.lastError().text());
        } else if (!query.exec("SELECT COUNT(*) FROM launches") || !query.next()) {
            label->setText("SQLite read failed: " + query.lastError().text());
        } else {
            label->setText("Qt " + QString::fromUtf8(qVersion()) + " + SQLite OK!\nLaunch count: " + query.value(0).toString() + "\nDatabase: " + db.databaseName());
        }
    }
    window.resize(470, 180);
    window.show();
    return app.exec();
}
