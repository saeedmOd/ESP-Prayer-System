// =====================================
// ESP Prayer System Web Controller
// =====================================

// =====================================
// Real-time Clock
// =====================================
function updateClock() {
    const now = new Date();

    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    const seconds = String(now.getSeconds()).padStart(2, '0');

    setText("time", `${hours}:${minutes}:${seconds}`);

    const date = now.toLocaleDateString("ar-EG", {
        weekday: "long",
        year: "numeric",
        month: "long",
        day: "numeric"
    });

    setText("date", date);
}

setInterval(updateClock, 1000);
updateClock();

// =====================================
// Countdown Global State
// =====================================
let countdownTimer = null;
let countdownSeconds = 0;

// =====================================
// Load ESP Status
// =====================================
function loadStatus() {
    // مهلة زمنية للطلب لمنع تراكم الـ Requests في حال بطء الـ WiFi
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), 4000);

    fetch("/api/status", { signal: controller.signal })
        .then(response => {
            clearTimeout(timeoutId);
            if (!response.ok) throw new Error("Network response was not ok");
            return response.json();
        })
        .then(data => {
            // Prayer Times
            setText("fajr", data.fajr);
            setText("dhuhr", data.dhuhr);
            setText("asr", data.asr);
            setText("maghrib", data.maghrib);
            setText("isha", data.isha);

            // Next Prayer
            setText("nextPrayer", data.nextPrayer);
            setText("nextPrayerTime", data.nextPrayerTime);

            // Countdown Logic
            if (data.countdown !== undefined && data.countdown !== null) {
                let newSeconds = Number(data.countdown);

                // Re-sync if first load or if drift exceeds 10s
                if (countdownTimer === null || Math.abs(newSeconds - countdownSeconds) > 10) {
                    countdownSeconds = newSeconds;

                    if (countdownTimer) {
                        clearInterval(countdownTimer);
                    }

                    updateCountdown();
                    countdownTimer = setInterval(updateCountdown, 1000);
                }
            }

            // System Status
            setText("wifi", data.wifi ? "متصل" : "غير متصل");
            setText("mqtt", data.mqtt ? "متصل" : "غير متصل");
            
            // Sync Volume Controls (Text & Slider)
            setText("volume", data.volume);
            setInputValue("volumeSlider", data.volume);
        })
        .catch(error => {
            console.warn("ESP Offline or connection error:", error);
            setText("wifi", "غير متصل");
            setText("mqtt", "غير متصل");
        });
}

// =====================================
// Countdown Timer Execution
// =====================================
function updateCountdown() {
    if (countdownSeconds <= 0) {
        setText("countdown", "حان الآن وقت الصلاة");
        if (countdownTimer) {
            clearInterval(countdownTimer);
            countdownTimer = null;
        }
        return;
    }

    const hours = Math.floor(countdownSeconds / 3600);
    const minutes = Math.floor((countdownSeconds % 3600) / 60);
    const seconds = countdownSeconds % 60;

    const formattedTime = [
        String(hours).padStart(2, '0'),
        String(minutes).padStart(2, '0'),
        String(seconds).padStart(2, '0')
    ].join(':');

    setText("countdown", "بعد " + formattedTime);

    countdownSeconds--;
}

// =====================================
// Safe DOM Helpers
// =====================================
function setText(id, value) {
    const element = document.getElementById(id);
    if (element && value !== undefined && value !== null) {
        element.textContent = value;
    }
}

function setInputValue(id, value) {
    const element = document.getElementById(id);
    // عدم التحديث إذا كان المستخدم يسحب شريط الصوت حالياً
    if (element && value !== undefined && document.activeElement !== element) {
        element.value = value;
    }
}

// =====================================
// Actions & API Callers
// =====================================
function testAzan() {
    fetch("/api/test/azan", { method: "POST" })
        .then(response => {
            if (response.ok) {
                alert("تم تشغيل الأذان التجريبي");
            } else {
                alert("فشل في إرسال الأمر للجهاز");
            }
        })
        .catch(() => {
            alert("الجهاز غير متصل");
        });
}

function saveVolume(value) {
    // تحديث النتيجة فوراً في الواجهة لتفاعل أسرع للمستخدم
    setText("volume", value);

    fetch("/api/settings/volume", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({ volume: Number(value) })
    })
    .catch(err => console.error("Error saving volume:", err));
}

// Sync status every 5 seconds
setInterval(loadStatus, 5000);
loadStatus();