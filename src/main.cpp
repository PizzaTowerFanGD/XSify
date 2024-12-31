#include <Geode/modify/LevelInfoLayer.hpp>
#include "cvoltonLevelTime.cpp"
#include <cmath>
#include <thread>

using namespace geode::prelude;

int logBaseN(float x, float n) {
    return std::log(x) / std::log(n);
}

std::string createXLstring(float levelLengthMinutes) {
    std::stringstream XLstring;
    int maximumXs = Mod::get()->getSettingValue<int64_t>("maximum-xs");
    bool usingPowerNotation = Mod::get()->getSettingValue<bool>("use-power-notation");
    bool usingXXLplus = Mod::get()->getSettingValue<bool>("xxl-plus");
    float xxlScaling = Mod::get()->getSettingValue<double>("xxl-scaling");
    int lengthExponent = logBaseN(levelLengthMinutes / 2, xxlScaling) + 1;

    float levelLengthSeconds = levelLengthMinutes * 60.0f;

    // Define the new level length categories
    if (levelLengthSeconds < 3.0f) {
        XLstring << "XXXXS";
    } else if (levelLengthSeconds < 5.0f) {
        XLstring << "XXXS";
    } else if (levelLengthSeconds < 7.0f) {
        XLstring << "XXS";
    } else if (levelLengthSeconds < 10.0f) {
        XLstring << "XS";
    } else if (levelLengthSeconds < 15.0f) {
        XLstring << "Tiny";
    } else if (levelLengthSeconds < 20.0f) {
        XLstring << "Tiny+";
    } else if (levelLengthSeconds < 25.0f) {
        XLstring << "Short-";
    } else if (levelLengthSeconds < 30.0f) {
        XLstring << "Short";
    } else if (levelLengthSeconds < 45.0f) {
        XLstring << "Short+";
    } else if (levelLengthSeconds < 60.0f) {
        XLstring << "Medium-";
    } else if (levelLengthSeconds < 90.0f) {
        XLstring << "Medium";
    } else if (levelLengthSeconds < 120.0f) {
        XLstring << "Medium+";
    } else if (levelLengthSeconds < 150.0f) {
        XLstring << "Long-";
    } else if (levelLengthSeconds < 180.0f) {
        XLstring << "Long";
    } else if (levelLengthSeconds < 240.0f) {
        XLstring << "Long+";
    } 
if (levelLengthSeconds < 240.0f) {
        return XLstring.str();
} else {
        if (usingPowerNotation && lengthExponent > maximumXs) {
            XLstring << "X^" << std::to_string(lengthExponent);
        } else {
            if (lengthExponent > maximumXs) {
                lengthExponent = maximumXs;
            }
            for (int i = 0; i < lengthExponent; i++) {
                if (i % 10 == 0 && i != 0) XLstring << "\n";
                XLstring << "X";
            }
        }
        XLstring << "L";

        if (usingXXLplus) {
            float XXLPlusLength = pow(xxlScaling, lengthExponent) + pow(xxlScaling, lengthExponent - 1);
            if (levelLengthMinutes >= XXLPlusLength) {
                XLstring << "+";
            }
        }
    }

    return XLstring.str();
}

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    void modifyXLlabel(float levelLengthMinutes) {
        bool usingCol = Mod::get()->getSettingValue<bool>("use-color");
        std::string XLstring = createXLstring(levelLengthMinutes);
        int XLstringLines = std::count(XLstring.begin(), XLstring.end(), '\n') + 1;
        
        m_lengthLabel->setString(XLstring.c_str());

        // linewrapping needs to reset anchor point
        if (XLstringLines > 1) {
            m_lengthLabel->setAnchorPoint({0, 1});
            m_lengthLabel->setPositionY(m_lengthLabel->getPositionY() + 8.125f);
        }

        if (usingCol) {
            int maxCol = Mod::get()->getSettingValue<int64_t>("maximum-color");
            ccColor3B labelCol = Mod::get()->getSettingValue<ccColor3B>("label-color");
            float colT = levelLengthMinutes < maxCol ? (float) levelLengthMinutes / maxCol : 1;

            labelCol.r = std::lerp(255, labelCol.r, colT);
            labelCol.g = std::lerp(255, labelCol.g, colT);
            labelCol.b = std::lerp(255, labelCol.b, colT);

            m_lengthLabel->setColor({labelCol.r, labelCol.g, labelCol.b});
        }
    }

    // Using CVolton's time calculator for levels before 2.2
    void createXLlabelCvolton() {
        this->retain();
        
        std::thread([this]() {
            thread::setName("CVoltonTime");
            float cvoltonLengthMinutes = timeForLevelString(m_level->m_levelString);

            Loader::get()->queueInMainThread([this, cvoltonLengthMinutes]() {
                modifyXLlabel(cvoltonLengthMinutes);
                this->release();
            });
        }).detach();
    }

    void createXLlabel() {
        if (m_level->isPlatformer()) {
            return;
        }
        
        float levelLengthMinutes = (float) m_level->m_timestamp / 14400.0f;
        
        if (levelLengthMinutes <= 0.0f) {
            createXLlabelCvolton();
            return;
        }

        modifyXLlabel(levelLengthMinutes);
    }

    void setupLevelInfo() {
        LevelInfoLayer::setupLevelInfo();
        MyLevelInfoLayer::createXLlabel();
    }

    virtual void levelDownloadFinished(GJGameLevel* p0) {
        LevelInfoLayer::levelDownloadFinished(p0);
        MyLevelInfoLayer::createXLlabel();
    }
};
