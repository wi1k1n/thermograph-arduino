// This is the exact same header-only lib as on gihub link below, however,
// enhanced by replacing magic numbers with macros and added state() func
/*
    Библиотека с логикой обработки кнопки (виртуальная кнопка)
    Документация:
    GitHub: https://github.com/GyverLibs/VirtualButton
    Возможности:
    - Очень лёгкий оптимизированный код
    - Множество сценариев использования
    - Позволяет расширить функционал других библиотек
    - Обработка:
        - Антидребезг
        - Нажатие
        - Отпускание
        - Клик
        - Несколько кликов
        - Счётчик кликов
        - Удержание
        - Импульсное удержание
        - Действия с предварительными кликами
    
    AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License
    
    v1.1 - добавлен механизм "таймаута"
*/
#ifndef _VirtualButton_h
#define _VirtualButton_h

// ========= НАСТРОЙКИ (можно передефайнить из скетча) ==========
#define _VB_DEB 50          // дебаунс кнопки, мс
#define _VB_CLICK 400	    // таймаут накликивания, мс

// =========== НЕ ТРОГАЙ ============
#include <Arduino.h>

#ifndef VB_DEB
#define VB_DEB _VB_DEB
#endif
#ifndef VB_CLICK
#define VB_CLICK _VB_CLICK
#endif

#define _VBFLAG_CLICK               0       // 0 - click
#define _VBFLAG_HELD                1       // 1 - held
#define _VBFLAG_STEP                2       // 2 - step
#define _VBFLAG_PRESS               3       // 3 - press
#define _VBFLAG_HOLD                4       // 4 - hold
#define _VBFLAG_CLICKS              5       // 5 - clicks flag
#define _VBFLAG_CLICKSGET           6       // 6 - clicks get
#define _VBFLAG_CLICKSGETNUM        7       // 7 - clicks get num
#define _VBFLAG_BUTTON              8       // 8 - флаг кнопки
#define _VBFLAG_BUSY                9       // 9 - busy flag
#define _VBFLAG_RELEASED            10      // 10 - btn released
#define _VBFLAG_LEVEL               11      // 11 - btn level
#define _VBFLAG_RELEASEDAFTERSTEP   12      // 12 - btn released after step
#define _VBFLAG_STEPFLAG            13      // 13 - step flag
#define _VBFLAG_DEBOUNCE            14      // 14 - deb flag
#define _VBFLAG_TIMEOUT             15      // 15 - timeout

// ======================================= CLASS =======================================
class VButton {
public:
    // таймаут удержания кнопки для hold(), 32.. 8100 мс (по умолч. 1000 мс)
    void setHoldTimeout(uint16_t tout) {
        _holdT = tout >> 5;
    }
    // период импульсов step(), 32.. 8100 мс (по умолч. 500 мс)
    void setStepTimeout(uint16_t tout) {
        _stepT = tout >> 5;
    }
    // опрос, вернёт true если статус кнопки изменился. Принимает состояние кнопки (1 - нажата)
    bool poll(bool s) {
        uint16_t prev = _flags;
        if (s || readF(_VBFLAG_BUSY))
            pollBtn(s); // опрос если кнопка нажата или не вышли таймауты
        return (prev != _flags);
    }
    // сбросить все флаги
    void reset() {
        _flags = 0;
    }

    // ======================================= BTN =======================================
	bool down() { return readF(_VBFLAG_BUTTON); }                       // нажата ли кнопка (false до дебаунса, true - все время после, если нажата)
    bool busy() { return readF(_VBFLAG_BUSY); }                         // вернёт true, если всё ещё нужно вызывать tick для опроса таймаутов
    bool press() { return checkF(_VBFLAG_PRESS); }                      // кнопка нажата
    bool release() { return checkF(_VBFLAG_RELEASED); }                 // кнопка отпущена
    bool click() { return checkF(_VBFLAG_CLICK); }                      // клик по кнопке
    
    bool held() { return checkF(_VBFLAG_HELD); }                        // кнопка удержана
    bool hold() { return readF(_VBFLAG_HOLD); }                         // кнопка удерживается
    bool step() { return checkF(_VBFLAG_STEP); }                        // режим импульсного удержания
    bool releaseStep() {
        return checkF(_VBFLAG_RELEASEDAFTERSTEP);                       // кнопка отпущена после импульсного удержания
    }
    
    bool held(uint8_t clk) { return (clicks == clk) ? held() : 0; }     // кнопка удержана с предварительным накликиванием
    bool hold(uint8_t clk) { return (clicks == clk) ? hold() : 0; }     // кнопка удерживается с предварительным накликиванием
    bool step(uint8_t clk) { return (clicks == clk) ? step() : 0; }     // режим импульсного удержания с предварительным накликиванием
    bool releaseStep(uint8_t clk) { return (clicks == clk) ? releaseStep() : 0; } // кнопка отпущена после импульсного удержания с предварительным накликиванием
    
    bool hasClicks(uint8_t num) { return (clicks == num && checkF(_VBFLAG_CLICKSGETNUM)) ? 1 : 0; } // имеются клики
    uint8_t hasClicks() { return checkF(_VBFLAG_CLICKSGET) ? clicks : 0; }                          // сколько имеется кликов
    
    // с момента отпускания кнопки прошло указанное время, миллисекунд
    bool timeout(uint16_t tout) { return ((uint16_t)(millis() & 0xFFFF) - _debTmr > tout && checkF(_VBFLAG_TIMEOUT)); }
    
    uint8_t clicks = 0;                                                 // счётчик кликов
    
private:
    // ===================================== POOL BTN =====================================
    void pollBtn(bool state) {
        uint16_t ms = millis() & 0xFFFF;
        uint16_t debounce = ms - _debTmr;
        if (state) {                                                // кнопка нажата
            setF(_VBFLAG_BUSY);                                     // busy флаг
            if (!readF(_VBFLAG_BUTTON)) {                           // и не была нажата ранее
                if (readF(_VBFLAG_DEBOUNCE)) {                      // ждём дебаунс
                    if (debounce > VB_DEB) {                        // прошел дебаунс
                        _flags |= 1 << _VBFLAG_BUTTON
                                | 1 << _VBFLAG_PRESS; // set 8 3 кнопка нажата
                        _debTmr = ms;                               // сброс таймаутов
                    }
                } else {                                            // первое нажатие
                    setF(_VBFLAG_DEBOUNCE);                         // запомнили что хотим нажать                    
                    if (debounce > VB_CLICK || readF(_VBFLAG_CLICKS)) { // кнопка нажата после VB_CLICK
                        clicks = 0;                                 // сбросить счётчик и флаг кликов
                        _flags &= ~(1 << _VBFLAG_CLICK              // clear 0 1 2 3 5 6 7 12 13
                                  | 1 << _VBFLAG_HELD
                                  | 1 << _VBFLAG_STEP
                                  | 1 << _VBFLAG_PRESS
                                  | 1 << _VBFLAG_CLICKS
                                  | 1 << _VBFLAG_CLICKSGET
                                  | 1 << _VBFLAG_CLICKSGETNUM
                                  | 1 << _VBFLAG_RELEASEDAFTERSTEP
                                  | 1 << _VBFLAG_STEPFLAG);
                    }
                    _debTmr = ms;
                }
            } else {                                                // кнопка уже была нажата
                if (!readF(_VBFLAG_HOLD)) {                         // и удержание ещё не зафиксировано
                    if (debounce >= (uint16_t)(_holdT << 5)) {      // прошло больше удержания
                        _flags |= 1 << _VBFLAG_HELD                 // set 1 4 5 запомнили что удерживается и отключаем сигнал о кликах
                                | 1 << _VBFLAG_HOLD
                                | 1 << _VBFLAG_CLICKS;
                        _debTmr = ms;                               // сброс таймаута
                    }
                } else {                                            // удержание зафиксировано
                    if (debounce > (uint16_t)(_stepT << 5)) {       // таймер степа
                        _flags |= 1 << _VBFLAG_STEP                 // set 2 13 step
                                | 1 << _VBFLAG_STEPFLAG;
                        _debTmr = ms;                               // сброс таймаута
                    }
                }
            }
        } else {                                                    // кнопка не нажата
            if (readF(_VBFLAG_BUTTON)) {                            // но была нажата
                if (debounce > VB_DEB) {
                    if (!readF(_VBFLAG_HOLD)) {                     // не удерживали - это клик
                        setF(_VBFLAG_CLICK);                        // click
                        clicks++;
                    }   
                    _flags &= ~(1 << _VBFLAG_BUTTON                 // clear 8 4
                              | 1 << _VBFLAG_HOLD);
                    _debTmr = ms;                                   // сброс таймаута
                    _flags |= 1 << _VBFLAG_RELEASED                 // set 10 15
                            | 1 << _VBFLAG_TIMEOUT;
                    if (checkF(_VBFLAG_STEPFLAG))
                        setF(_VBFLAG_RELEASEDAFTERSTEP);            // кнопка отпущена после step
                }
            } else if (clicks && !readF(_VBFLAG_CLICKS)) {          // есть клики
                if (debounce > VB_CLICK)
                    _flags |= 1 << _VBFLAG_CLICKS                     // set 5 6 7 (клики)
                            | 1 << _VBFLAG_CLICKSGET
                            | 1 << _VBFLAG_CLICKSGETNUM;
            } else {
                clrF(_VBFLAG_BUSY);                                 // снимаем busy флаг
            }
            checkF(_VBFLAG_DEBOUNCE);                               // сброс ожидания нажатия
        }
    }
    
    // ===================================== MISC =====================================
    bool checkF(const uint8_t val) { return readF(val) ? clrF(val), 1 : 0; }
    inline void setF(const uint8_t x) __attribute__((always_inline)) { _flags |= 1 << x; }
    inline void clrF(const uint8_t x) __attribute__((always_inline)) { _flags &= ~(1 << x); }
    inline bool readF(const uint8_t x) __attribute__((always_inline)) { return _flags & (1 << x); }

    uint16_t _flags = 0;
    uint8_t _holdT = 1000 >> 5;
    uint8_t _stepT = 500 >> 5;
    uint16_t _debTmr = 0;
};
#endif