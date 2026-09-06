//  OptimCompare - Project #16 (F. Piralić, 20106)
//  ToolBar.h
#pragma once
#include <gui/ToolBar.h>
#include <gui/Image.h>
#include "Constants.h"

class ToolBar : public gui::ToolBar
{
    gui::Image _imgSettings;
    gui::Image _imgRun;

public:
    ToolBar()
    : gui::ToolBar("mainTB", 3)
    , _imgSettings(":settings")
    , _imgRun(":start")
    {
        addItem(tr("settings"), &_imgSettings, tr("settingsTT"), cMenuApp, 0, 0, 10);
        addItem(tr("run"), &_imgRun, tr("runTT"), cMenuRun, 0, 0, cActionRun);
        addItem(tr("runStudy"), &_imgRun, tr("runStudyTT"), cMenuRun, 0, 0, cActionStudy);
    }
};
