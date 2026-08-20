# دليل تعديل التصميم في EasyEDA

## المشكلة
ملف CSV ما يُستورد مباشرة في EasyEDA.

## الحل: تعديل التصميم الموجود يدوياً

### الخطوة 1: افتح ملف التصميم
1. افتح EasyEDA (Desktop أو Web)
2. افتح ملف `PCB_PCB_Dfplayer_Athan_mini-copy_2026-08-18.json`

---

### الخطوة 2: فعّل Net Labels
في EasyEDA:
1. اضغط `N` على الكيبورد (أو اختر Net Label من الشريط)
2. هذا يظهر أسماء الـ nets على الأسلاك

---

### الخطوة 3: التوصيلات المطلوبة (Final Assignments)

#### الأطراف اللي تتغير:

| المكون | الكائن القديم | الكائن الجديد | الشبكة (Net) |
|--------|---------------|---------------|--------------|
| Stop Button | D1 (GPIO5) | **D2 (GPIO4)** | `GPIO4` |
| Buzzer | D2 (GPIO4) | **D3 (GPIO0)** | `FLASH` |
| Rotary DT | D3 (GPIO0) | **D8 (GPIO15)** | `GPIO15` |

#### الأطراف اللي ما تتغير:

| المكون | الأطراف | الشبكة (Net) |
|--------|---------|--------------|
| LCD SCL | D0 (GPIO16) | `GPIO16` |
| LCD SDA | D1 (GPIO5) | `GPIO5` |
| DFPlayer RX | D6 (GPIO12) | `GPIO12` |
| DFPlayer TX | D5 (GPIO14) | `GPIO14` |
| Rotary CLK | D7 (GPIO13) | `GPIO13` |
| Rotary SW | D4 (GPIO2) | `GPIO2` |

---

### الخطوة 4: كيف تعدّل التوصيلات

#### 4.1 Stop Button → D2 (GPIO4)
```
القديم: Stop Button على D1 (GPIO5)
الجديد: Stop Button على D2 (GPIO4)

في EasyEDA:
1. احذف السلك من D1
2. اسحب سلك جديد من Button إلى GPIO4
3. أو غيّر اسم الـ Net Label من "GPIO5" إلى "GPIO4"
```

#### 4.2 Buzzer → D3 (GPIO0)
```
القديم: Buzzer على D2 (GPIO4)
الجديد: Buzzer على D3 (GPIO0)

في EasyEDA:
1. احذف السلك من D2
2. اسحب سلك جديد من Buzzer إلى GPIO0
3. أو غيّر اسم الـ Net Label من "GPIO4" إلى "FLASH"

ملاحظة: Buzzer يحتاج transistor driver:
- GPIO0 → resistor 1kΩ → transistor base
- collector → buzzer → +5V
- emitter → GND
```

#### 4.3 Rotary DT → D8 (GPIO15)
```
القديم: Rotary DT على D3 (GPIO0)
الجديد: Rotary DT على D8 (GPIO15)

في EasyEDA:
1. احذف السلك من D3
2. اسحب سلك جديد من Rotary DT إلى GPIO15
3. أو غيّر اسم الـ Net Label من "FLASH" إلى "GPIO15"
```

---

### الخطوة 5: تأكد من التوصيلات النهائية

| ESP Pin | GPIO | Net Name | الوظيفة |
|---------|------|----------|---------|
| D0 | GPIO16 | `GPIO16` | LCD SCL |
| D1 | GPIO5 | `GPIO5` | LCD SDA |
| D2 | GPIO4 | `GPIO4` | Stop Button |
| D3 | GPIO0 | `FLASH` | Buzzer |
| D4 | GPIO2 | `GPIO2` | Rotary SW |
| D5 | GPIO14 | `GPIO14` | DFPlayer TX |
| D6 | GPIO12 | `GPIO12` | DFPlayer RX |
| D7 | GPIO13 | `GPIO13` | Rotary CLK |
| D8 | GPIO15 | `GPIO15` | Rotary DT |

---

### ملاحظات مهمة

1. **GPIO15 (D8)**: يحتاج pulldown 10kΩ أثناء الـ boot
   - تأكد من وجود resistor من GPIO15 إلى GND

2. **GPIO0 (D3)**: آمن للـ output بعد الـ boot
   - يُستخدم للـ Buzzer عبر transistor

3. **GPIO16 (D0)**: ما عنده قيود boot
   - يُستخدم لـ LCD SCL

4. **GPIO4 و GPIO5**: مشتركان بين LCD والدوائر الجديدة
   - تأكد من أن الـ pull-up على I2C كافية

---

### اختصار سريع في EasyEDA

بplaced تحذف وتسحب أسلاك جديدة، يمكنك:

1. **استخدم Net Labels**: اضغط `N` وضع label جديد
2. **غيّر أسماء الـ Nets**: اضغط على الـ label واكتب الاسم الجديد
3. **تأكد من الاتصال**: كل الـ nets بنفس الاسم تتصل تلقائياً

---

## أو: أنشئ ملف EasyEDA جديد

إذا كنت تفضل تبدأ من جديد:

1. افتح EasyEDA Schematic Editor
2. اضغط `Ctrl+I` لاستيراد
3. اختر "Import Other EDA File"
4. اختر ملف JSON القديم كمرجع
5. أنشئ المخطط الجديد بالظبط
