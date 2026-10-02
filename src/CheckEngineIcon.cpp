#include "CheckEngineIcon.h"
#include <QByteArray>

CheckEngineIcon::CheckEngineIcon(QWidget* parent) : QSvgWidget(parent) {
    setFixedSize(64, 46);
    render("#465363");
}

void CheckEngineIcon::setActive(bool active) {
    if (active == active_) return;
    active_ = active;
    render(active ? "#ffb33e" : "#465363");
}

void CheckEngineIcon::render(const QString& color) {
    const QString svg = QString(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 80 54'>"
        "<g fill='none' stroke='%1' stroke-width='2.8' stroke-linejoin='round' stroke-linecap='round'>"
        "<path d='M15 18h9l5-7h18l6 7h10v10h7l5-5v22l-5-5h-7v7H25l-10-9z'/>"
        "<path d='M30 6h15M37 6v5M8 23v14M8 30h7'/>"
        "</g></svg>").arg(color);
    load(svg.toUtf8());
}
