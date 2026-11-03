#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QResizeEvent>
#include <QVector>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QSpinBox>
#include <QStatusBar>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <algorithm>
#include <cmath>

class MarkdownHighlighter final : public QSyntaxHighlighter {
public:
    explicit MarkdownHighlighter(QTextDocument *document) : QSyntaxHighlighter(document) {
        heading_.setForeground(QColor("#3979ad"));
        heading_.setFontWeight(QFont::Bold);
        bold_.setFontWeight(QFont::Bold);
        italic_.setFontItalic(true);
        code_.setForeground(QColor("#9360a6"));
        code_.setFontFamilies({"Consolas", "Courier New", "monospace"});
        link_.setForeground(QColor("#2679ad"));
        link_.setFontUnderline(true);
        marker_.setForeground(QColor("#5d8a6e"));
        quote_.setForeground(QColor("#7b8292"));
        quote_.setFontItalic(true);
    }
protected:
    void highlightBlock(const QString &s) override {
        const bool fence = QRegularExpression(R"(^\s*(```|~~~))").match(s).hasMatch();
        if (fence) {
            setFormat(0, s.size(), code_);
            setCurrentBlockState(previousBlockState() == 1 ? 0 : 1);
            return;
        }
        if (previousBlockState() == 1) {
            setFormat(0, s.size(), code_);
            setCurrentBlockState(1);
            return;
        }
        setCurrentBlockState(0);
        apply(s, QRegularExpression(R"(^\s{0,3}#{1,6}\s+.*$)"), heading_);
        apply(s, QRegularExpression(R"((\*\*|__)(?=\S).+?\1)"), bold_);
        apply(s, QRegularExpression(R"((?<!\*)\*(?!\*)(?=\S).+?\*(?!\*)|(?<!_)_(?!_)(?=\S).+?_(?!_))"), italic_);
        apply(s, QRegularExpression(R"(`[^`]+`)"), code_);
        apply(s, QRegularExpression(R"(\[[^\]]+\]\([^\)]+\))"), link_);
        apply(s, QRegularExpression(R"(^\s*(?:[-+*]|\d+\.)\s+)"), marker_);
        apply(s, QRegularExpression(R"(^\s*>.*$)"), quote_);
    }
private:
    void apply(const QString &s, const QRegularExpression &re, const QTextCharFormat &format) {
        auto it = re.globalMatch(s);
        while (it.hasNext()) {
            const auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), format);
        }
    }
    QTextCharFormat heading_, bold_, italic_, code_, link_, marker_, quote_;
};

class AutoHeightEditor final : public QPlainTextEdit {
public:
    explicit AutoHeightEditor(QWidget *parent = nullptr) : QPlainTextEdit(parent) {
        setPlaceholderText("Supporting notes… (grows with content)");
        setMinimumHeight(58);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        connect(document(), &QTextDocument::contentsChanged, this, [this] { updateHeight(); });
    }

    void setHeightCap(int maxPixels) {
        cap_ = std::max(58, maxPixels);
        updateHeight();
    }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QPlainTextEdit::resizeEvent(event);
        updateHeight();
    }
private:
    void updateHeight() {
        QTextDocument measure;
        measure.setDefaultFont(font());

        QTextOption options = document()->defaultTextOption();
        measure.setDefaultTextOption(options);

        // Use the editor's available text width.
        const int textWidth = viewport()->width();
        measure.setTextWidth(textWidth);

        measure.setPlainText(toPlainText());

        // QTextDocument uses pixel dimensions here.
        const double contentHeight = measure.size().height();

        // Account for the editor frame and internal margins.
        const int extraHeight = 2 * frameWidth() + 4;

        const int desired =
            qCeil(contentHeight) + extraHeight;

        const int target = qBound(58, desired, cap_);

        if (height() != target)
            setFixedHeight(target);
    }
    int cap_ = 250;
};

class MainWindow final : public QMainWindow {
public:
    MainWindow() : settings_("RationalePad", "RationalePad") {
        setWindowTitle("RationalePad");
        resize(980, 720);

        auto *central = new QWidget(this);
        auto *layout = new QVBoxLayout(central);
        layout->setContentsMargins(14, 12, 14, 12);
        layout->setSpacing(10);

        auto *timerPanel = new QWidget(central);
        auto *timerLayout = new QVBoxLayout(timerPanel);
        timerLayout->setContentsMargins(0, 0, 0, 0);
        timerLayout->setSpacing(8);
        display_ = new QLabel("25:00", timerPanel);
        QFont timerFont = display_->font();
        timerFont.setPointSize(31);
        timerFont.setBold(true);
        display_->setFont(timerFont);
        display_->setAlignment(Qt::AlignHCenter);
        timerLayout->addWidget(display_);

        auto *controls = new QHBoxLayout();
        for (int i = 0; i < 4; ++i) {
            auto *button = new QPushButton(timerPanel);
            button->setMinimumWidth(85);
            recentButtons_.append(button);
            controls->addWidget(button);
            connect(button, &QPushButton::clicked, this, [this, i] { startDuration(recents_[i]); });
        }
        auto *other = new QPushButton("Other…", timerPanel);
        controls->addWidget(other);
        controls->addStretch(1);
        pause_ = new QPushButton("Start", timerPanel);
        reset_ = new QPushButton("Reset", timerPanel);
        controls->addWidget(pause_);
        controls->addWidget(reset_);
        timerLayout->addLayout(controls);
        layout->addWidget(timerPanel);

        auto *topLabel = new QLabel("Main editor", central);
        layout->addWidget(topLabel);
        mainEditor_ = new QPlainTextEdit(central);
        mainEditor_->setPlaceholderText("Start writing…");
        mainEditor_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        layout->addWidget(mainEditor_, 1);
        highlighter_ = new MarkdownHighlighter(mainEditor_->document());

        auto *bottomLabel = new QLabel("Supporting notes", central);
        layout->addWidget(bottomLabel);
        bottom_ = new AutoHeightEditor(central);
        bottom_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        layout->addWidget(bottom_);
        setCentralWidget(central);

        auto *view = menuBar()->addMenu("&View");
        auto *markdownAction = view->addAction("&Markdown highlighting");
        markdownAction->setCheckable(true);
        markdownAction->setChecked(true);
        connect(markdownAction, &QAction::toggled, highlighter_, [this](bool enabled) {
            highlighter_->setDocument(enabled ? mainEditor_->document() : nullptr);
        });
        statusBar()->showMessage("RationalePad UI prototype — text is not saved yet");

        QVariantList stored = settings_.value("timer/recentDurations").toList();
        for (const auto &v : stored) {
            const int n = v.toInt();
            if (n > 0 && !recents_.contains(n)) recents_.append(n);
            if (recents_.size() == 4) break;
        }
        for (int n : {5*60, 15*60, 25*60, 45*60}) {
            if (recents_.size() == 4) break;
            if (!recents_.contains(n)) recents_.append(n);
        }
        refreshRecents();
        remainingMs_ = 25 * 60 * 1000LL;
        updateClock();

        connect(other, &QPushButton::clicked, this, [this] { chooseCustomDuration(); });
        connect(pause_, &QPushButton::clicked, this, [this] { togglePause(); });
        connect(reset_, &QPushButton::clicked, this, [this] { resetTimer(); });
        tick_.setInterval(100);
        connect(&tick_, &QTimer::timeout, this, [this] { updateClock(); });

        // Recompute cap whenever the central area changes size.
        central->installEventFilter(this);
        updateBottomCap();
    }
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == centralWidget() && event->type() == QEvent::Resize)
            QTimer::singleShot(0, this, [this] { updateBottomCap(); });
        return QMainWindow::eventFilter(obj, event);
    }
private:
    void updateBottomCap() {
        if (!centralWidget()) return;
        // Percentage of space available for both editors after timer and labels.
        const int available = std::max(160, centralWidget()->height() - mainEditor_->y());
        bottom_->setHeightCap(static_cast<int>(available * 0.40));
    }
    static QString durationLabel(int seconds) {
        if (seconds % 3600 == 0) return QString::number(seconds / 3600) + " h";
        if (seconds % 60 == 0) return QString::number(seconds / 60) + " min";
        return QString::number(seconds) + " sec";
    }
    void refreshRecents() {
        for (int i = 0; i < recentButtons_.size(); ++i)
            recentButtons_[i]->setText(durationLabel(recents_[i]));
    }
    void startDuration(int seconds) {
        if (seconds <= 0) return;
        selectedSeconds_ = seconds;
        remainingMs_ = static_cast<qint64>(seconds) * 1000;
        recents_.removeAll(seconds);
        recents_.prepend(seconds);
        while (recents_.size() > 4) recents_.removeLast();
        QVariantList stored;
        for (int n : recents_) stored << n;
        settings_.setValue("timer/recentDurations", stored);
        refreshRecents();
        running_ = true;
        elapsed_.start();
        tick_.start();
        pause_->setText("Pause");
        updateClock();
    }
    void chooseCustomDuration() {
        QDialog dialog(this);
        dialog.setWindowTitle("Set task time");
        QFormLayout form(&dialog);
        QSpinBox hours, minutes, seconds;
        hours.setRange(0, 99);
        minutes.setRange(0, 59);
        seconds.setRange(0, 59);
        minutes.setValue(25);
        form.addRow("Hours", &hours);
        form.addRow("Minutes", &minutes);
        form.addRow("Seconds", &seconds);
        QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        form.addRow(&buttons);
        connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() == QDialog::Accepted) {
            const int total = hours.value() * 3600 + minutes.value() * 60 + seconds.value();
            if (total > 0) startDuration(total);
        }
    }
    void togglePause() {
        if (running_) {
            remainingMs_ = std::max<qint64>(0, remainingMs_ - elapsed_.elapsed());
            running_ = false;
            tick_.stop();
            pause_->setText("Resume");
        } else if (remainingMs_ > 0) {
            running_ = true;
            elapsed_.start();
            tick_.start();
            pause_->setText("Pause");
        } else {
            startDuration(selectedSeconds_);
        }
        updateClock();
    }
    void resetTimer() {
        running_ = false;
        tick_.stop();
        remainingMs_ = static_cast<qint64>(selectedSeconds_) * 1000;
        pause_->setText("Start");
        updateClock();
    }
    void updateClock() {
        qint64 ms = remainingMs_;
        if (running_) ms = std::max<qint64>(0, remainingMs_ - elapsed_.elapsed());
        if (running_ && ms == 0) {
            running_ = false;
            remainingMs_ = 0;
            tick_.stop();
            pause_->setText("Start");
            QApplication::beep();
            statusBar()->showMessage("Time is up");
        }
        const qint64 whole = (ms + 999) / 1000;
        const qint64 h = whole / 3600;
        const qint64 m = (whole / 60) % 60;
        const qint64 s = whole % 60;
        display_->setText(h > 0
            ? QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'))
            : QString("%1:%2").arg(whole / 60, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    }
    QSettings settings_;
    QLabel *display_ = nullptr;
    QPushButton *pause_ = nullptr;
    QPushButton *reset_ = nullptr;
    QVector<QPushButton *> recentButtons_;
    QVector<int> recents_;
    QPlainTextEdit *mainEditor_ = nullptr;
    AutoHeightEditor *bottom_ = nullptr;
    MarkdownHighlighter *highlighter_ = nullptr;
    QTimer tick_;
    QElapsedTimer elapsed_;
    qint64 remainingMs_ = 0;
    int selectedSeconds_ = 25 * 60;
    bool running_ = false;
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setOrganizationName("RationalePad");
    app.setApplicationName("RationalePad");
    MainWindow window;
    window.show();
    return app.exec();
}
