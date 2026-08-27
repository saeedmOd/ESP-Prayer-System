// =====================================
// ESP Prayer System Web Controller
// script.js
// =====================================

console.log(
    "ESP Prayer System script.js LOADED"
);


// =====================================
// Global State
// =====================================

let timeFormat = "24H";

let countdownTimer = null;

let countdownSeconds = 0;


// =====================================
// Arabic Prayer Names
// =====================================

const prayerNamesArabic = {

    Fajr:
        "الفجر",

    Sunrise:
        "الشروق",

    Dhuhr:
        "الظهر",

    Asr:
        "العصر",

    Maghrib:
        "المغرب",

    Isha:
        "العشاء"

};


// =====================================
// Safe DOM Helpers
// =====================================

function setText(
    id,
    value
) {

    const element =
        document.getElementById(id);


    if (
        element &&
        value !== undefined &&
        value !== null
    ) {

        element.textContent =
            value;

    }

}


function setInputValue(
    id,
    value
) {

    const element =
        document.getElementById(id);


    if (
        element &&
        value !== undefined &&
        document.activeElement !== element
    ) {

        element.value =
            value;

    }

}


// =====================================
// Real-time Clock
// =====================================

function updateClock() {

    const now =
        new Date();


    let hours =
        now.getHours();


    const minutes =
        String(
            now.getMinutes()
        ).padStart(
            2,
            "0"
        );


    const seconds =
        String(
            now.getSeconds()
        ).padStart(
            2,
            "0"
        );


    let timeString;


    // =====================================
    // 12 Hour
    // =====================================

    if (
        timeFormat === "12H"
    ) {

        const period =
            hours >= 12
                ? "PM"
                : "AM";


        hours =
            hours % 12;


        if (
            hours === 0
        ) {

            hours = 12;

        }


        timeString =
            String(hours).padStart(
                2,
                "0"
            ) +
            ":" +
            minutes +
            ":" +
            seconds +
            " " +
            period;

    }


    // =====================================
    // 24 Hour
    // =====================================

    else {

        timeString =
            String(hours).padStart(
                2,
                "0"
            ) +
            ":" +
            minutes +
            ":" +
            seconds;

    }


    setText(
        "time",
        timeString
    );


    // =====================================
    // Date
    // =====================================

    const date =
        now.toLocaleDateString(
            "ar-EG",
            {

                year:
                    "numeric",

                month:
                    "long",

                day:
                    "numeric"

            }
        );


    setText(
        "date",
        date
    );

}


// =====================================
// Start Clock
// =====================================

updateClock();


setInterval(
    updateClock,
    1000
);


// =====================================
// Load ESP Status
// =====================================

async function loadStatus() {

    console.log(
        "[STATUS] Requesting /api/status"
    );


    const controller =
        new AbortController();


    const timeoutId =
        setTimeout(
            () => {

                controller.abort();

            },
            10000
        );


    try {

        const response =
            await fetch(
                "/api/status",
                {

                    signal:
                        controller.signal,

                    cache:
                        "no-store"

                }
            );


        clearTimeout(
            timeoutId
        );


        if (
            !response.ok
        ) {

            throw new Error(
                "HTTP " +
                response.status
            );

        }


        const data =
            await response.json();


        console.log(
            "[STATUS]",
            data
        );


        // =================================
        // Time Format
        // =================================

        if (
            data.timeFormat === "12H" ||
            data.timeFormat === "24H"
        ) {

            timeFormat =
                data.timeFormat;


            updateClock();

        }


        // =================================
        // Prayer Times
        // =================================

        setText(
            "fajr",
            data.fajr
        );


        setText(
            "dhuhr",
            data.dhuhr
        );


        setText(
            "asr",
            data.asr
        );


        setText(
            "maghrib",
            data.maghrib
        );


        setText(
            "isha",
            data.isha
        );


        setText(
            "iqamaFajr",
            data.iqamaFajr
                ? "اق " + data.iqamaFajr
                : ""
        );

        setText(
            "iqamaDhuhr",
            data.iqamaDhuhr
                ? "اق " + data.iqamaDhuhr
                : ""
        );

        setText(
            "iqamaAsr",
            data.iqamaAsr
                ? "اق " + data.iqamaAsr
                : ""
        );

        setText(
            "iqamaMaghrib",
            data.iqamaMaghrib
                ? "اق " + data.iqamaMaghrib
                : ""
        );

        setText(
            "iqamaIsha",
            data.iqamaIsha
                ? "اق " + data.iqamaIsha
                : ""
        );


        // =================================
        // Next Prayer
        // =================================

        const nextPrayer =
            prayerNamesArabic[
                data.nextPrayer
            ] ||
            data.nextPrayer ||
            "---";


        setText(
            "nextPrayer",
            nextPrayer
        );


        // =================================
        // Next Prayer Time
        // =================================

        setText(
            "nextPrayerTime",
            data.nextPrayerTime ||
            "--:--"
        );


        // =================================
        // Countdown
        // =================================

        updateCountdownFromStatus(
            data.countdown
        );


        // =================================
        // Event Status Banner
        // =================================

        updateEventBanner(data);


        // =================================
        // Wi-Fi
        // =================================

        setText(
            "wifi",
            data.wifi
                ? "متصل"
                : "غير متصل"
        );


        // =================================
        // MQTT
        // =================================

        setText(
            "mqtt",
            data.mqtt
                ? "متصل"
                : "غير متصل"
        );


        // =================================
        // Device Status
        // =================================

        updateDeviceStatus(
            data.status
        );


        // =================================
        // DFPlayer
        // =================================

        if (
            data.playerReady !== undefined
        ) {

            setText(
                "dfplayer",
                data.playerReady
                    ? "متصل"
                    : "غير متصل"
            );

        }


        // =================================
        // Volume
        // =================================

        setText(
            "volume",
            data.volume
        );


        setInputValue(
            "volumeSlider",
            data.volume
        );


        // =================================
        // Hijri Date
        // =================================

        if (data.hijri) {
            setText("hijriDate", data.hijri);
        }

    }


    catch (error) {

        clearTimeout(
            timeoutId
        );


        console.warn(
            "[STATUS ERROR]",
            error
        );


        handleStatusError();

    }

}


// =====================================
// Countdown Status Update
// =====================================

function updateCountdownFromStatus(
    value
) {

    if (
        value === undefined ||
        value === null
    ) {

        return;

    }


    const newSeconds =
        Number(value);


    if (
        !Number.isFinite(
            newSeconds
        ) ||
        newSeconds < 0
    ) {

        return;

    }


    if (isSettingsPage) {
        return;
    }


    countdownSeconds =
        Math.floor(
            newSeconds
        );


    updateCountdown();


    /*
     * تأكد أن هناك Timer واحد فقط.
     */

    if (
        countdownTimer === null
    ) {

        countdownTimer =
            setInterval(
                updateCountdown,
                1000
            );

    }

}


// =====================================
// Event Status Banner
// =====================================

function updateEventBanner(
    data
) {

    const banner =
        document.getElementById(
            "eventBanner"
        );

    if (!banner) {
        return;
    }


    if (
        data.eventActive
    ) {

        let html =
            "🕌 الآن: " +
            (data.eventTitle || "");

        if (
            data.eventSubtitle
        ) {
            html +=
                ' <span class="event-subtitle">' +
                data.eventSubtitle +
                "</span>";
        }

        banner.innerHTML = html;

        banner.style.display = "flex";

    } else {

        banner.style.display = "none";

    }

}


// =====================================
// Device Status
// =====================================

function updateDeviceStatus(
    status
) {

    const element =
        document.getElementById(
            "deviceStatus"
        );


    if (!element) {
        return;
    }


    if (
        status === "online"
    ) {

        element.textContent =
            "متصل";

        element.className =
            "status-online";

    }


    else {

        element.textContent =
            "غير متصل";

        element.className =
            "status-offline";

    }

}


// =====================================
// Status Error
// =====================================

function handleStatusError() {

    setText(
        "wifi",
        "غير متصل"
    );


    setText(
        "mqtt",
        "غير متصل"
    );


    setText(
        "dfplayer",
        "غير متصل"
    );


    setText(
        "nextPrayer",
        "---"
    );


    setText(
        "nextPrayerTime",
        "--:--"
    );


    updateDeviceStatus(
        "offline"
    );

}


// =====================================
// Countdown Timer
// =====================================

function updateCountdown() {

    const element =
        document.getElementById(
            "countdown"
        );


    if (!element) {

        console.warn(
            "[COUNTDOWN] Element not found"
        );

        return;

    }


    // =====================================
    // Prayer Time Now
    // =====================================

    if (
        countdownSeconds <= 0
    ) {

        element.textContent =
            "حان الآن وقت الصلاة";


        if (
            countdownTimer !== null
        ) {

            clearInterval(
                countdownTimer
            );


            countdownTimer =
                null;

        }


        return;

    }


    // =====================================
    // Calculate Time
    // =====================================

    const hours =
        Math.floor(
            countdownSeconds /
            3600
        );


    const minutes =
        Math.floor(
            (
                countdownSeconds %
                3600
            ) / 60
        );


    const seconds =
        countdownSeconds %
        60;


    const formattedTime =

        String(hours).padStart(
            2,
            "0"
        ) +
        ":" +
        String(minutes).padStart(
            2,
            "0"
        ) +
        ":" +
        String(seconds).padStart(
            2,
            "0"
        );


    element.textContent =
        "بعد " +
        formattedTime;


    countdownSeconds--;

}


// =====================================
// Initial Load
// =====================================

loadStatus();
loadLastEvent();


// =====================================
// Sync ESP Status Every 5 Seconds
// =====================================

setInterval(
    loadStatus,
    5000
);


// =====================================
// Event Log - Last Event Bar
// =====================================

function loadLastEvent()
{
    fetch("/api/logs?limit=1")
    .then(function(r) { return r.json(); })
    .then(function(data)
    {
        if (!data || data.length === 0)
            return;

        var e = data[0];
        var bar = document.getElementById("eventBar");
        var txt = document.getElementById("eventBarText");

        if (!bar || !txt)
            return;

        var t = new Date(e.ts * 1000);
        var time = t.getHours().toString().padStart(2,"0") + ":" + t.getMinutes().toString().padStart(2,"0");

        var statusIcon = "✅";
        if (e.status === "fail") statusIcon = "❌";
        if (e.status === "skip") statusIcon = "⚠️";

        txt.textContent = time + " — " + e.action + " (" + e.src + ") " + statusIcon;
        bar.style.display = "flex";
    })
    .catch(function() {});
}

setInterval(loadLastEvent, 10000);


/* ==========================================================
   SETTINGS PAGE — Combined JS
   ========================================================== */


/* ==========================================================
   DEFAULTS
   ========================================================== */

const DEFAULTS = {

    volume: 1,

    azan: {
        enable: true,
        device: 0,
        buzzerTone: 0,
        folder: 1,
        file: 1
    },

    iqama: {
        enable: false,
        device: 0,
        buzzerTone: 0,
        folder: 1,
        file: 4,
        volume: 1,
        delay: 10,
        fajr: false,
        dhuhr: false,
        asr: false,
        maghrib: false,
        isha: false
    },

    morningAdhkar: {
        enable: false,
        hour: 6,
        minute: 0,
        volume: 1,
        folder: 4,
        file: 1
    },

    eveningAdhkar: {
        enable: false,
        hour: 18,
        minute: 0,
        volume: 1,
        folder: 4,
        file: 2
    },

    kahf: {
        enable: false,
        hour: 9,
        minute: 0,
        volume: 1,
        folder: 5,
        file: 1
    },

    alarmToneType: 0,

    customAlert: {
        enable: false,
        source: 0,
        hour: 0,
        minute: 0,
        days: 127,
        repeat: 0,
        interval: 1,
        file: 1,
        volume: 1
    },

    quran: {
        baqarah: {
            enable: false, hour: 0, minute: 0, volume: 1, folder: 1, file: 1
        },
        baqarahLast: {
            enable: false, hour: 0, minute: 0, volume: 1, folder: 1, file: 1
        },
        ayatKursi: {
            enable: false, hour: 0, minute: 0, volume: 1, folder: 1, file: 1
        },
        maryam: {
            enable: false, hour: 0, minute: 0, volume: 1, folder: 1, file: 1
        }
    },

    folderPlay: {
        category: 1,
        volume: 1,
        mode: "sequential"
    }
};


/* ==========================================================
   STATE
   ========================================================== */

let audioSettings = deepClone(DEFAULTS);
let isLoading = false;
let isSavingNetwork = false;
let audioControlBusy = false;
let testBusy = false;
let toastTimer = null;
let isSettingsPage = false;


/* ==========================================================
   HELPERS
   ========================================================== */

function deepClone(obj) {
    return JSON.parse(JSON.stringify(obj));
}

function $(id) {
    return document.getElementById(id);
}

function setNumber(id, value) {
    const el = $(id);
    if (el) el.value = Number.isFinite(Number(value)) ? Number(value) : "";
}

function setSelect(id, value) {
    const el = $(id);
    if (!el) return;
    el.value = typeof value === "boolean" ? (value ? "true" : "false") : String(value);
}

function setText(id, value) {
    const el = $(id);
    if (el) el.textContent = value;
}

function setTextValue(id, value) {
    const el = $(id);
    if (el) el.value = value ?? "";
}

function readNumber(id, fallback = 0) {
    const el = $(id);
    if (!el) return fallback;
    const v = Number(el.value);
    return Number.isFinite(v) ? v : fallback;
}

function readBool(id, fallback = false) {
    const el = $(id);
    if (!el) return fallback;
    return el.value === "true" || el.value === "1";
}

function readSelect(id, fallback = 0) {
    const el = $(id);
    if (!el) return fallback;
    const v = Number(el.value);
    return Number.isFinite(v) ? v : fallback;
}

function clamp(value, min, max) {
    const n = Number(value);
    if (!Number.isFinite(n)) return min;
    return Math.max(min, Math.min(max, n));
}

function setChecked(id, value) {
    const el = $(id);
    if (el) el.checked = Boolean(value);
}

function getChecked(id) {
    const el = $(id);
    return el ? el.checked : false;
}


/* ==========================================================
   TOAST
   ========================================================== */

function showToast(message, type = "success") {
    const toast = $("toast");
    if (!toast) return;
    clearTimeout(toastTimer);
    toast.textContent = message;
    toast.className = "toast " + type + " show";
    toastTimer = setTimeout(() => {
        toast.classList.remove("show");
    }, 2800);
}


/* ==========================================================
   STATUS BADGE
   ========================================================== */

function setStatus(text, type = "") {
    const el = $("status");
    if (!el) return;
    el.textContent = text;
    el.classList.remove("success", "error");
    if (type) el.classList.add(type);
}


/* ==========================================================
   TAB NAVIGATION
   ========================================================== */

function initTabs() {
    const saved = localStorage.getItem("settingsTab") || "prayer";

    document.querySelectorAll(".tab-btn").forEach(btn => {
        btn.addEventListener("click", () => {
            switchTab(btn.dataset.tab);
        });
    });

    switchTab(saved);
}

function switchTab(tabId) {
    document.querySelectorAll(".tab-btn").forEach(b => {
        b.classList.toggle("active", b.dataset.tab === tabId);
    });
    document.querySelectorAll(".tab-content").forEach(c => {
        c.classList.toggle("active", c.id === "tab-" + tabId);
    });
    localStorage.setItem("settingsTab", tabId);

    /* Show/hide per-tab save buttons */
    const showPrayer = tabId === "prayer";
    const showAlert = tabId === "alert";
    const showAdhkar = tabId === "adhkar";
    const showQuran = tabId === "quran";
    const showNetwork = tabId === "network";

    if ($("savePrayerBtn")) $("savePrayerBtn").style.display = showPrayer ? "" : "none";
    if ($("saveAlertBtn")) $("saveAlertBtn").style.display = showAlert ? "" : "none";
    if ($("saveAdhkarBtn")) $("saveAdhkarBtn").style.display = showAdhkar ? "" : "none";
    if ($("saveQuranBtn")) $("saveQuranBtn").style.display = showQuran ? "" : "none";
    if ($("saveNetworkBtn")) $("saveNetworkBtn").style.display = showNetwork ? "" : "none";
}


/* ==========================================================
   VOLUME DISPLAYS
   ========================================================== */

function updateVolumeDisplays() {
    const mappings = [
        ["volume", "volumeValue"],
        ["iqamaVolume", "iqamaVolumeValue"],
        ["morningAdhkarVolume", "morningAdhkarVolumeValue"],
        ["eveningAdhkarVolume", "eveningAdhkarVolumeValue"],
        ["kahfVolume", "kahfVolumeValue"],
        ["quranVolume", "quranVolumeValue"],
        ["customAlertVolume", "customAlertVolumeValue"]
    ];
    mappings.forEach(([inputId, outputId]) => {
        const input = $(inputId);
        const output = $(outputId);
        if (input && output) output.textContent = input.value;
    });
}


/* ==========================================================
   ENABLE / DISABLE VISUALS
   ========================================================== */

function updateEnabledVisual(selectId, cardId) {
    const select = $(selectId);
    const card = $(cardId);
    if (!select || !card) return;
    card.classList.toggle("disabled", select.value !== "true");
}


function updateDeviceSections() {
    const azanDevice = $("azanDevice");
    const azanDfplayer = $("azanDfplayerSection");
    const azanBuzzer = $("azanBuzzerSection");
    if (azanDevice) {
        const isBuzzer = azanDevice.value === "1";
        if (azanDfplayer) azanDfplayer.style.display = isBuzzer ? "none" : "";
        if (azanBuzzer) azanBuzzer.style.display = isBuzzer ? "" : "none";
    }

    const iqamaDevice = $("iqamaDevice");
    const iqamaDfplayer = $("iqamaDfplayerSection");
    const iqamaBuzzer = $("iqamaBuzzerSection");
    if (iqamaDevice) {
        const isBuzzer = iqamaDevice.value === "1";
        if (iqamaDfplayer) iqamaDfplayer.style.display = isBuzzer ? "none" : "";
        if (iqamaBuzzer) iqamaBuzzer.style.display = isBuzzer ? "" : "none";
    }
}


/* ==========================================================
   PRESET HELPERS
   ========================================================== */

function readPreset(id, fallbackFolder, fallbackFile) {
    const el = $(id);
    if (!el) return { folder: fallbackFolder, file: fallbackFile };
    const parts = String(el.value).split(":");
    const folder = Number(parts[0]);
    const file = Number(parts[1]);
    return {
        folder: Number.isFinite(folder) && folder > 0 ? folder : fallbackFolder,
        file: Number.isFinite(file) && file > 0 ? file : fallbackFile
    };
}

function setPreset(id, folder, file) {
    const el = $(id);
    if (!el) return;
    const value = Number(folder) + ":" + Number(file);
    const exists = [...el.options].some(o => o.value === value);
    if (exists) el.value = value;
}


/* ==========================================================
   CUSTOM ALERT HELPERS
   ========================================================== */

function collectCustomAlertDays() {
    let days = 0;
    for (let i = 0; i < 7; i++) {
        const cb = $("customAlertDays" + i);
        if (cb && cb.checked) days |= parseInt(cb.value);
    }
    return days;
}

function updateCustomAlertDays(days) {
    for (let i = 0; i < 7; i++) {
        const cb = $("customAlertDays" + i);
        if (cb) cb.checked = (days & parseInt(cb.value)) !== 0;
    }
}

function updateCustomAlertSourceUI(source) {
    const isBuzzer = source === 0;
    const buzzerSection = $("customAlertBuzzerSection");
    const fileSection = $("customAlertFileSection");
    if (buzzerSection) buzzerSection.style.display = isBuzzer ? "" : "none";
    if (fileSection) fileSection.style.display = isBuzzer ? "none" : "";
}

function updateCustomAlertRepeatUI(repeat) {
    const group = $("customAlertIntervalGroup");
    if (group) group.style.display = repeat > 0 ? "" : "none";
}


/* ==========================================================
   QURAN HELPERS
   ========================================================== */

function getSelectedQuran() {
    const select = $("quranType");
    if (!select) return null;
    const option = select.options[select.selectedIndex];
    if (!option) return null;
    return { type: option.value, folder: Number(option.dataset.folder), file: Number(option.dataset.file) };
}

function loadSelectedQuran() {
    const selected = getSelectedQuran();
    if (!selected) return;
    const info = $("quranFileInfo");
    if (info) info.textContent = "مجلد " + selected.folder + " / ملف " + selected.file;
    const settings = audioSettings.quran[selected.type];
    if (!settings) return;
    setSelect("quranEnable", settings.enable);
    setNumber("quranHour", settings.hour);
    setNumber("quranMinute", settings.minute);
    setNumber("quranVolume", settings.volume);
    updateVolumeDisplays();
    updateEnabledVisual("quranEnable", "quranCard");
}

function readCurrentQuran() {
    const selected = getSelectedQuran();
    if (!selected) return;
    if (!audioSettings.quran[selected.type]) {
        audioSettings.quran[selected.type] = { ...selected, enable: true, hour: 0, minute: 0, volume: 10 };
    }
    const q = audioSettings.quran[selected.type];
    q.enable = readBool("quranEnable", true);
    q.hour = clamp(readNumber("quranHour", 0), 0, 23);
    q.minute = clamp(readNumber("quranMinute", 0), 0, 59);
    q.volume = clamp(readNumber("quranVolume", 10), 0, 30);
    q.folder = selected.folder;
    q.file = selected.file;
}


/* ==========================================================
   AUDIO CONTROL
   ========================================================== */

function setAudioControlState(text) {
    const el = $("audioControlState");
    if (el) el.textContent = text;
}

function setAudioButtonsDisabled(disabled) {
    ["audioPlayButton", "audioPauseButton", "audioStopButton", "audioVolumeUpButton", "audioVolumeDownButton"].forEach(id => {
        const btn = $(id);
        if (btn) btn.disabled = disabled;
    });
}

async function audioRequest(endpoint, successMessage) {
    if (audioControlBusy) return;
    audioControlBusy = true;
    setAudioButtonsDisabled(true);
    setAudioControlState("جاري التنفيذ...");
    try {
        const response = await fetch(endpoint, { method: "POST", cache: "no-store" });
        const text = await response.text();
        if (!response.ok) throw new Error("HTTP " + response.status + " " + text);
        let result = null;
        try { result = JSON.parse(text); } catch (_) { result = null; }
        if (result && result.volume !== undefined) {
            setNumber("volume", clamp(result.volume, 0, 30));
            updateVolumeDisplays();
        }
        setAudioControlState(successMessage);
        showToast(successMessage, "success");
    } catch (error) {
        console.error("[AUDIO CONTROL]", error);
        setAudioControlState("خطأ");
        showToast("فشل تنفيذ أمر الصوت", "error");
    } finally {
        audioControlBusy = false;
        setAudioButtonsDisabled(false);
        setTimeout(() => { if (!audioControlBusy) setAudioControlState("جاهز"); }, 1800);
    }
}

function audioPlay() { audioRequest("/api/audio?action=play", "▶️ تم التشغيل"); }
function audioPause() { audioRequest("/api/audio?action=pause", "⏸️ تم الإيقاف المؤقت"); }
function audioStop() { audioRequest("/api/audio?action=stop", "⏹️ تم الإيقاف"); }
function audioVolumeUp() { audioRequest("/api/audio?action=volume-up", "🔊 تم رفع الصوت"); }
function audioVolumeDown() { audioRequest("/api/audio?action=volume-down", "🔉 تم خفض الصوت"); }


/* ==========================================================
   TEST ENDPOINTS
   ========================================================== */

async function testEndpoint(type, successMessage, button = null) {
    if (testBusy) return;
    testBusy = true;
    if (button) button.disabled = true;
    try {
        const response = await fetch("/api/test?type=" + encodeURIComponent(type), { method: "POST", cache: "no-store" });
        const text = await response.text();
        if (!response.ok) throw new Error("HTTP " + response.status);
        showToast(successMessage, "success");
    } catch (error) {
        console.error("[TEST]", error);
        showToast("فشل التشغيل", "error");
    } finally {
        testBusy = false;
        if (button) button.disabled = false;
    }
}

async function testQuran() {
    const selected = getSelectedQuran();
    if (!selected) { showToast("لم يتم اختيار محتوى القرآن", "warning"); return; }
    const volume = clamp(readNumber("quranVolume", 10), 0, 30);
    const button = $("quranTestButton");
    if (button) button.disabled = true;
    try {
        const response = await fetch("/api/test/quran", {
            method: "POST", cache: "no-store",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ type: selected.type, folder: selected.folder, file: selected.file, volume: volume })
        });
        if (!response.ok) throw new Error("HTTP " + response.status);
        showToast("تم تشغيل المحتوى المختار", "success");
    } catch (error) {
        console.error("[QURAN TEST]", error);
        showToast("فشل تشغيل المحتوى المختار", "error");
    } finally {
        if (button) button.disabled = false;
    }
}

async function playFolder() {
    const categorySelect = $("folderCategory");
    const folder = clamp(Number(categorySelect?.value || 1), 1, 99);
    const fileCount = clamp(Number(categorySelect?.selectedOptions[0]?.dataset?.count || 5), 1, 99);
    const volume = clamp(Number($("folderVolume")?.value || 15), 0, 30);
    const mode = document.querySelector('input[name="folderMode"]:checked')?.value || "sequential";

    const button = $("folderButton");
    const stopBtn = $("folderStopButton");
    if (button) button.disabled = true;
    try {
        const response = await fetch("/api/test/folder", {
            method: "POST", cache: "no-store",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ folder: folder, volume: volume, fileCount: fileCount, mode: mode })
        });
        if (!response.ok) throw new Error("HTTP " + response.status);
        showToast("تم تشغيل المجلد " + folder, "success");
        if (stopBtn) stopBtn.style.display = "";
    } catch (error) {
        console.error("[FOLDER]", error);
        showToast("فشل تشغيل المجلد", "error");
    } finally {
        if (button) button.disabled = false;
    }
}


async function stopFolder() {
    const stopBtn = $("folderStopButton");
    if (stopBtn) stopBtn.disabled = true;
    try {
        const response = await fetch("/api/audio?action=stop", {
            method: "POST", cache: "no-store"
        });
        if (!response.ok) throw new Error("HTTP " + response.status);
        showToast("تم إيقاف التشغيل", "success");
        if (stopBtn) stopBtn.style.display = "none";
    } catch (error) {
        console.error("[STOP]", error);
        showToast("فشل الإيقاف", "error");
    } finally {
        if (stopBtn) stopBtn.disabled = false;
    }
}


/* ==========================================================
   LOAD AUDIO SETTINGS
   ========================================================== */

async function loadAudioSettings() {
    if (isLoading) return;
    isLoading = true;
    setStatus("جاري التحميل...");
    try {
        const response = await fetch("/api/settings/audio", { method: "GET", cache: "no-store" });
        if (!response.ok) throw new Error("HTTP " + response.status);
        const data = await response.json();
        audioSettings = deepClone(DEFAULTS);

        audioSettings.volume = clamp(data.volume ?? DEFAULTS.volume, 0, 30);
        setNumber("volume", audioSettings.volume);

        audioSettings.azan.enable = data.azanEnable ?? DEFAULTS.azan.enable;
        audioSettings.azan.device = data.azanDevice ?? DEFAULTS.azan.device;
        audioSettings.azan.buzzerTone = data.azanBuzzerTone ?? DEFAULTS.azan.buzzerTone;
        audioSettings.azan.folder = data.azanFolder ?? DEFAULTS.azan.folder;
        audioSettings.azan.file = data.azanFile ?? DEFAULTS.azan.file;
        setSelect("azanEnable", audioSettings.azan.enable);
        setSelect("azanDevice", audioSettings.azan.device);
        setSelect("azanBuzzerTone", audioSettings.azan.buzzerTone);
        setPreset("azanPreset", audioSettings.azan.folder, audioSettings.azan.file);
        updateDeviceSections();

        audioSettings.iqama.enable = data.iqamaEnable ?? DEFAULTS.iqama.enable;
        audioSettings.iqama.device = data.iqamaDevice ?? DEFAULTS.iqama.device;
        audioSettings.iqama.buzzerTone = data.iqamaBuzzerTone ?? DEFAULTS.iqama.buzzerTone;
        audioSettings.iqama.folder = data.iqamaFolder ?? DEFAULTS.iqama.folder;
        audioSettings.iqama.file = data.iqamaFile ?? DEFAULTS.iqama.file;
        audioSettings.iqama.volume = clamp(data.iqamaVolume ?? DEFAULTS.iqama.volume, 0, 30);
        audioSettings.iqama.delay = clamp(data.iqamaDelay ?? DEFAULTS.iqama.delay, 0, 60);
        audioSettings.iqama.fajr = data.iqamaFajr ?? DEFAULTS.iqama.fajr;
        audioSettings.iqama.dhuhr = data.iqamaDhuhr ?? DEFAULTS.iqama.dhuhr;
        audioSettings.iqama.asr = data.iqamaAsr ?? DEFAULTS.iqama.asr;
        audioSettings.iqama.maghrib = data.iqamaMaghrib ?? DEFAULTS.iqama.maghrib;
        audioSettings.iqama.isha = data.iqamaIsha ?? DEFAULTS.iqama.isha;
        setSelect("iqamaEnable", audioSettings.iqama.enable);
        setSelect("iqamaDevice", audioSettings.iqama.device);
        setSelect("iqamaBuzzerTone", audioSettings.iqama.buzzerTone);
        setPreset("iqamaPreset", audioSettings.iqama.folder, audioSettings.iqama.file);
        setNumber("iqamaVolume", audioSettings.iqama.volume);
        setNumber("iqamaDelay", audioSettings.iqama.delay);
        setChecked("iqamaFajr", audioSettings.iqama.fajr);
        setChecked("iqamaDhuhr", audioSettings.iqama.dhuhr);
        setChecked("iqamaAsr", audioSettings.iqama.asr);
        setChecked("iqamaMaghrib", audioSettings.iqama.maghrib);
        setChecked("iqamaIsha", audioSettings.iqama.isha);

        setNumber("iqamaFajrDelay", clamp(data.iqamaFajrDelay ?? 20, 0, 180));
        setNumber("iqamaDhuhrDelay", clamp(data.iqamaDhuhrDelay ?? 10, 0, 180));
        setNumber("iqamaAsrDelay", clamp(data.iqamaAsrDelay ?? 10, 0, 180));
        setNumber("iqamaMaghribDelay", clamp(data.iqamaMaghribDelay ?? 5, 0, 180));
        setNumber("iqamaIshaDelay", clamp(data.iqamaIshaDelay ?? 10, 0, 180));

        audioSettings.morningAdhkar.enable = data.morningAdhkarEnable ?? DEFAULTS.morningAdhkar.enable;
        audioSettings.morningAdhkar.hour = clamp(data.morningAdhkarHour ?? DEFAULTS.morningAdhkar.hour, 0, 23);
        audioSettings.morningAdhkar.minute = clamp(data.morningAdhkarMinute ?? DEFAULTS.morningAdhkar.minute, 0, 59);
        audioSettings.morningAdhkar.volume = clamp(data.morningAdhkarVolume ?? DEFAULTS.morningAdhkar.volume, 0, 30);
        audioSettings.morningAdhkar.folder = data.morningAdhkarFolder ?? DEFAULTS.morningAdhkar.folder;
        audioSettings.morningAdhkar.file = data.morningAdhkarFile ?? DEFAULTS.morningAdhkar.file;
        setSelect("morningAdhkarEnable", audioSettings.morningAdhkar.enable);
        setNumber("morningAdhkarHour", audioSettings.morningAdhkar.hour);
        setNumber("morningAdhkarMinute", audioSettings.morningAdhkar.minute);
        setNumber("morningAdhkarVolume", audioSettings.morningAdhkar.volume);

        audioSettings.eveningAdhkar.enable = data.eveningAdhkarEnable ?? DEFAULTS.eveningAdhkar.enable;
        audioSettings.eveningAdhkar.hour = clamp(data.eveningAdhkarHour ?? DEFAULTS.eveningAdhkar.hour, 0, 23);
        audioSettings.eveningAdhkar.minute = clamp(data.eveningAdhkarMinute ?? DEFAULTS.eveningAdhkar.minute, 0, 59);
        audioSettings.eveningAdhkar.volume = clamp(data.eveningAdhkarVolume ?? DEFAULTS.eveningAdhkar.volume, 0, 30);
        audioSettings.eveningAdhkar.folder = data.eveningAdhkarFolder ?? DEFAULTS.eveningAdhkar.folder;
        audioSettings.eveningAdhkar.file = data.eveningAdhkarFile ?? DEFAULTS.eveningAdhkar.file;
        setSelect("eveningAdhkarEnable", audioSettings.eveningAdhkar.enable);
        setNumber("eveningAdhkarHour", audioSettings.eveningAdhkar.hour);
        setNumber("eveningAdhkarMinute", audioSettings.eveningAdhkar.minute);
        setNumber("eveningAdhkarVolume", audioSettings.eveningAdhkar.volume);

        audioSettings.kahf.enable = data.kahfEnable ?? DEFAULTS.kahf.enable;
        audioSettings.kahf.hour = clamp(data.kahfHour ?? DEFAULTS.kahf.hour, 0, 23);
        audioSettings.kahf.minute = clamp(data.kahfMinute ?? DEFAULTS.kahf.minute, 0, 59);
        audioSettings.kahf.volume = clamp(data.kahfVolume ?? DEFAULTS.kahf.volume, 0, 30);
        setSelect("kahfEnable", audioSettings.kahf.enable);
        setNumber("kahfHour", audioSettings.kahf.hour);
        setNumber("kahfMinute", audioSettings.kahf.minute);
        setNumber("kahfVolume", audioSettings.kahf.volume);
        let savedKahfFile = data.kahfFile;
        if (savedKahfFile === undefined || savedKahfFile === null) {
            savedKahfFile = (data.kahfFolder !== undefined && data.kahfFolder <= 4)
                ? data.kahfFolder
                : DEFAULTS.kahf.file;
        }
        setSelect("kahfReciter", savedKahfFile);

        setSelect("eidTakbeeratEnable", data.eidTakbeeratEnable ?? false);
        setNumber("eidTakbeeratVolume", data.eidTakbeeratVolume ?? 1);
        const etVal = $("eidTakbeeratVolumeValue");
        if (etVal) etVal.textContent = String(data.eidTakbeeratVolume ?? 1);

        setNumber("ruqyahFolder", data.ruqyahFolder ?? 6);
        setNumber("ruqyahFile", data.ruqyahFile ?? 1);
        setNumber("ruqyahVolume", data.ruqyahVolume ?? 15);
        const rqVal = $("ruqyahVolumeValue");
        if (rqVal) rqVal.textContent = String(data.ruqyahVolume ?? 15);

        setSelect("dhikrRepeatEnable", data.dhikrRepeatEnable ?? false);
        setNumber("dhikrRepeatInterval", data.dhikrRepeatInterval ?? 5);
        setNumber("dhikrRepeatVolume", data.dhikrRepeatVolume ?? 1);
        const drVal = $("dhikrRepeatVolumeValue");
        if (drVal) drVal.textContent = String(data.dhikrRepeatVolume ?? 1);

        audioSettings.alarmToneType = clamp(data.alarmToneType ?? DEFAULTS.alarmToneType, 0, 7);
        setSelect("alarmToneType", audioSettings.alarmToneType);

        audioSettings.customAlert.enable = data.customAlertEnable ?? DEFAULTS.customAlert.enable;
        audioSettings.customAlert.source = clamp(data.customAlertSource ?? DEFAULTS.customAlert.source, 0, 1);
        audioSettings.customAlert.hour = clamp(data.customAlertHour ?? DEFAULTS.customAlert.hour, 0, 23);
        audioSettings.customAlert.minute = clamp(data.customAlertMinute ?? DEFAULTS.customAlert.minute, 0, 59);
        audioSettings.customAlert.days = clamp(data.customAlertDays ?? DEFAULTS.customAlert.days, 0, 127);
        audioSettings.customAlert.repeat = clamp(data.customAlertRepeat ?? DEFAULTS.customAlert.repeat, 0, 4);
        audioSettings.customAlert.interval = clamp(data.customAlertInterval ?? DEFAULTS.customAlert.interval, 1, 60);
        audioSettings.customAlert.file = clamp(data.customAlertFile ?? DEFAULTS.customAlert.file, 1, 11);
        audioSettings.customAlert.volume = clamp(data.customAlertVolume ?? DEFAULTS.customAlert.volume, 0, 30);
        setSelect("customAlertEnable", audioSettings.customAlert.enable);
        setSelect("customAlertSource", audioSettings.customAlert.source);
        setNumber("customAlertHour", audioSettings.customAlert.hour);
        setNumber("customAlertMinute", audioSettings.customAlert.minute);
        updateCustomAlertDays(audioSettings.customAlert.days);
        setSelect("customAlertRepeat", audioSettings.customAlert.repeat);
        setNumber("customAlertInterval", audioSettings.customAlert.interval);
        setSelect("customAlertFile", audioSettings.customAlert.file);
        setNumber("customAlertVolume", audioSettings.customAlert.volume);
        updateCustomAlertSourceUI(audioSettings.customAlert.source);
        updateCustomAlertRepeatUI(audioSettings.customAlert.repeat);

        ["baqarah", "baqarahLast", "ayatKursi", "maryam"].forEach(key => {
            const q = (data.quran && data.quran[key]) ? data.quran[key] : null;
            if (!q) return;
            audioSettings.quran[key].enable = q.enable ?? DEFAULTS.quran[key].enable;
            audioSettings.quran[key].hour = clamp(q.hour ?? DEFAULTS.quran[key].hour, 0, 23);
            audioSettings.quran[key].minute = clamp(q.minute ?? DEFAULTS.quran[key].minute, 0, 59);
            audioSettings.quran[key].volume = clamp(q.volume ?? DEFAULTS.quran[key].volume, 0, 30);
            audioSettings.quran[key].folder = q.folder ?? DEFAULTS.quran[key].folder;
            audioSettings.quran[key].file = q.file ?? DEFAULTS.quran[key].file;
        });
        loadSelectedQuran();

        const fp = data.folderPlay || {};
        audioSettings.folderPlay.category = clamp(Number(fp.category ?? DEFAULTS.folderPlay.category), 1, 99);
        audioSettings.folderPlay.volume = clamp(Number(fp.volume ?? DEFAULTS.folderPlay.volume), 0, 30);
        audioSettings.folderPlay.mode = ["sequential", "shuffle", "loop"].includes(fp.mode)
            ? fp.mode
            : DEFAULTS.folderPlay.mode;

        setSelect("folderCategory", audioSettings.folderPlay.category);
        setNumber("folderVolume", audioSettings.folderPlay.volume);
        const fvVal = $("folderVolumeValue");
        if (fvVal) fvVal.textContent = String(audioSettings.folderPlay.volume);

        const modeRadio = document.querySelector('input[name="folderMode"][value="' + audioSettings.folderPlay.mode + '"]');
        if (modeRadio) modeRadio.checked = true;

        setStatus("متصل", "success");
    } catch (error) {
        console.error("[AUDIO] Load error:", error);
        setStatus("خطأ في الاتصال", "error");
    } finally {
        isLoading = false;
    }
}


/* ==========================================================
   COLLECT PER-TAB SETTINGS
   ========================================================== */

function collectPrayerTab() {
    const azan = readPreset("azanPreset", DEFAULTS.azan.folder, DEFAULTS.azan.file);
    const iqama = readPreset("iqamaPreset", DEFAULTS.iqama.folder, DEFAULTS.iqama.file);

    return {
        volume: clamp(readNumber("volume", DEFAULTS.volume), 0, 30),
        azanEnable: readBool("azanEnable", DEFAULTS.azan.enable),
        azanDevice: clamp(readNumber("azanDevice", DEFAULTS.azan.device), 0, 1),
        azanBuzzerTone: clamp(readNumber("azanBuzzerTone", DEFAULTS.azan.buzzerTone), 0, 7),
        azanFolder: azan.folder,
        azanFile: azan.file,
        iqamaEnable: readBool("iqamaEnable", DEFAULTS.iqama.enable),
        iqamaDevice: clamp(readNumber("iqamaDevice", DEFAULTS.iqama.device), 0, 1),
        iqamaBuzzerTone: clamp(readNumber("iqamaBuzzerTone", DEFAULTS.iqama.buzzerTone), 0, 7),
        iqamaFolder: iqama.folder,
        iqamaFile: iqama.file,
        iqamaVolume: clamp(readNumber("iqamaVolume", DEFAULTS.iqama.volume), 0, 30),
        iqamaDelay: clamp(readNumber("iqamaDelay", DEFAULTS.iqama.delay), 0, 60),
        iqamaFajr: getChecked("iqamaFajr"),
        iqamaDhuhr: getChecked("iqamaDhuhr"),
        iqamaAsr: getChecked("iqamaAsr"),
        iqamaMaghrib: getChecked("iqamaMaghrib"),
        iqamaIsha: getChecked("iqamaIsha"),
        iqamaFajrDelay: clamp(readNumber("iqamaFajrDelay", 20), 0, 180),
        iqamaDhuhrDelay: clamp(readNumber("iqamaDhuhrDelay", 10), 0, 180),
        iqamaAsrDelay: clamp(readNumber("iqamaAsrDelay", 10), 0, 180),
        iqamaMaghribDelay: clamp(readNumber("iqamaMaghribDelay", 5), 0, 180),
        iqamaIshaDelay: clamp(readNumber("iqamaIshaDelay", 10), 0, 180)
    };
}

function collectAlertTab() {
    return {
        customAlertEnable: readBool("customAlertEnable", DEFAULTS.customAlert.enable),
        customAlertSource: clamp(readSelect("customAlertSource", DEFAULTS.customAlert.source), 0, 1),
        customAlertHour: clamp(readNumber("customAlertHour", DEFAULTS.customAlert.hour), 0, 23),
        customAlertMinute: clamp(readNumber("customAlertMinute", DEFAULTS.customAlert.minute), 0, 59),
        customAlertDays: clamp(collectCustomAlertDays(), 0, 127),
        customAlertRepeat: clamp(readSelect("customAlertRepeat", DEFAULTS.customAlert.repeat), 0, 4),
        customAlertInterval: clamp(readNumber("customAlertInterval", DEFAULTS.customAlert.interval), 1, 60),
        customAlertFile: clamp(readSelect("customAlertFile", DEFAULTS.customAlert.file), 1, 11),
        customAlertVolume: clamp(readNumber("customAlertVolume", DEFAULTS.customAlert.volume), 0, 30),
        alarmToneType: clamp(readSelect("alarmToneType", DEFAULTS.alarmToneType), 0, 7)
    };
}

function collectAdhkarTab() {
    return {
        morningAdhkarEnable: readBool("morningAdhkarEnable", DEFAULTS.morningAdhkar.enable),
        morningAdhkarHour: clamp(readNumber("morningAdhkarHour", DEFAULTS.morningAdhkar.hour), 0, 23),
        morningAdhkarMinute: clamp(readNumber("morningAdhkarMinute", DEFAULTS.morningAdhkar.minute), 0, 59),
        morningAdhkarVolume: clamp(readNumber("morningAdhkarVolume", DEFAULTS.morningAdhkar.volume), 0, 30),
        morningAdhkarFolder: audioSettings.morningAdhkar?.folder ?? DEFAULTS.morningAdhkar.folder,
        morningAdhkarFile: audioSettings.morningAdhkar?.file ?? DEFAULTS.morningAdhkar.file,
        eveningAdhkarEnable: readBool("eveningAdhkarEnable", DEFAULTS.eveningAdhkar.enable),
        eveningAdhkarHour: clamp(readNumber("eveningAdhkarHour", DEFAULTS.eveningAdhkar.hour), 0, 23),
        eveningAdhkarMinute: clamp(readNumber("eveningAdhkarMinute", DEFAULTS.eveningAdhkar.minute), 0, 59),
        eveningAdhkarVolume: clamp(readNumber("eveningAdhkarVolume", DEFAULTS.eveningAdhkar.volume), 0, 30),
        eveningAdhkarFolder: audioSettings.eveningAdhkar?.folder ?? DEFAULTS.eveningAdhkar.folder,
        eveningAdhkarFile: audioSettings.eveningAdhkar?.file ?? DEFAULTS.eveningAdhkar.file,
        kahfEnable: readBool("kahfEnable", DEFAULTS.kahf.enable),
        kahfHour: clamp(readNumber("kahfHour", DEFAULTS.kahf.hour), 0, 23),
        kahfMinute: clamp(readNumber("kahfMinute", DEFAULTS.kahf.minute), 0, 59),
        kahfVolume: clamp(readNumber("kahfVolume", DEFAULTS.kahf.volume), 0, 30),
        kahfFolder: audioSettings.kahf?.folder ?? 5,
        kahfFile: clamp(readNumber("kahfReciter", DEFAULTS.kahf.file), 1, 4),
        eidTakbeeratEnable: readBool("eidTakbeeratEnable", false),
        eidTakbeeratVolume: clamp(readNumber("eidTakbeeratVolume", 1), 0, 30),
        ruqyahFolder: 6,
        ruqyahFile: clamp(readNumber("ruqyahFile", 1), 1, 255),
        ruqyahVolume: clamp(readNumber("ruqyahVolume", 15), 0, 30),
        dhikrRepeatEnable: readBool("dhikrRepeatEnable", false),
        dhikrRepeatInterval: clamp(readNumber("dhikrRepeatInterval", 5), 1, 60),
        dhikrRepeatVolume: clamp(readNumber("dhikrRepeatVolume", 1), 0, 30)
    };
}

function collectQuranTab() {
    readCurrentQuran();
    return {
        quran: deepClone(audioSettings.quran),
        folderPlay: {
            category: clamp(Number($("folderCategory")?.value || 1), 1, 99),
            volume: clamp(Number($("folderVolume")?.value || 15), 0, 30),
            mode: document.querySelector('input[name="folderMode"]:checked')?.value || "sequential"
        }
    };
}


/* ==========================================================
   SAVE PER-TAB
   ========================================================== */

async function saveSettingsTab(collectFn, endpoint, btnId, label) {
    const btn = $(btnId);
    if (btn) { btn.disabled = true; btn.textContent = "⏳ جاري الحفظ..."; }
    try {
        const data = collectFn();
        const response = await fetch(endpoint, {
            method: "POST", cache: "no-store",
            headers: { "Content-Type": "application/json", "Accept": "application/json" },
            body: JSON.stringify(data)
        });
        if (!response.ok) throw new Error("HTTP " + response.status);
        if (data.volume !== undefined) audioSettings.volume = data.volume;
        showToast("تم حفظ " + label + " بنجاح", "success");
    } catch (error) {
        console.error("[SAVE] Error:", error);
        showToast("فشل حفظ " + label, "error");
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = "💾 حفظ " + label; }
    }
}

async function savePrayerTab() {
    const btn = $("savePrayerBtn");
    if (btn) { btn.disabled = true; btn.textContent = "⏳ جاري الحفظ..."; }
    try {
        // Save prayer location/settings
        const prayerData = {
            prayer_source: $("prayerSource").value,
            city: $("city").value,
            country: $("country").value,
            latitude: parseFloat($("latitude").value),
            longitude: parseFloat($("longitude").value),
            method: $("method").value,
            time_format: $("timeFormat").value,
            fajr_offset: parseInt($("fajr_offset").value) || 0,
            dhuhr_offset: parseInt($("dhuhr_offset").value) || 0,
            asr_offset: parseInt($("asr_offset").value) || 0,
            maghrib_offset: parseInt($("maghrib_offset").value) || 0,
            isha_offset: parseInt($("isha_offset").value) || 0
        };
        const r1 = await fetch("/api/settings/prayer", {
            method: "POST", cache: "no-store",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(prayerData)
        });
        if (!r1.ok) throw new Error("HTTP " + r1.status);

        // Save azan/iqama audio settings
        const audioData = collectPrayerTab();
        const r2 = await fetch("/api/settings/audio", {
            method: "POST", cache: "no-store",
            headers: { "Content-Type": "application/json", "Accept": "application/json" },
            body: JSON.stringify(audioData)
        });
        if (!r2.ok) throw new Error("HTTP " + r2.status);

        audioSettings.volume = audioData.volume;
        showToast("تم حفظ إعدادات الصلاة بنجاح", "success");
    } catch (error) {
        console.error("[PRAYER] Save error:", error);
        showToast("فشل حفظ إعدادات الصلاة", "error");
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = "💾 حفظ الصلاة"; }
    }
}
async function saveAlertTab() { return saveSettingsTab(collectAlertTab, "/api/settings/audio", "saveAlertBtn", "التنبيه"); }
async function saveAdhkarTab() { return saveSettingsTab(collectAdhkarTab, "/api/settings/audio", "saveAdhkarBtn", "الأذكار"); }
async function saveQuranTab() { return saveSettingsTab(collectQuranTab, "/api/settings/audio", "saveQuranBtn", "القرآن"); }


/* ==========================================================
   LOAD PRAYER SETTINGS
   ========================================================== */

async function loadPrayerSettings() {
    try {
        const response = await fetch("/api/settings/prayer", { method: "GET", cache: "no-store" });
        if (!response.ok) throw new Error("HTTP " + response.status);
        const data = await response.json();
        setTextValue("city", data.city ?? "");
        setTextValue("country", data.country ?? "");
        setNumber("latitude", data.latitude ?? "");
        setNumber("longitude", data.longitude ?? "");
        setSelect("method", data.method ?? data.calculation_method ?? "UmmAlQura");
        setSelect("prayerSource", data.prayer_source ?? "local");
        setSelect("timeFormat", data.time_format ?? data.timeFormat ?? "24H");
        setNumber("fajr_offset", data.fajr_offset ?? 0);
        setNumber("dhuhr_offset", data.dhuhr_offset ?? 0);
        setNumber("asr_offset", data.asr_offset ?? 0);
        setNumber("maghrib_offset", data.maghrib_offset ?? 0);
        setNumber("isha_offset", data.isha_offset ?? 0);
    } catch (error) {
        console.error("[PRAYER] Load error:", error);
    }
}


/* ==========================================================
   SAVE PRAYER
   ========================================================== */

/* ==========================================================
   WIFI SCANNING
   ========================================================== */

let wifiScanTimer = null;

async function startWifiScan() {
    const btn = $("wifiScanBtn");
    const list = $("wifiNetworkList");
    if (!btn || !list) return;
    btn.disabled = true;
    btn.textContent = "🔍 جاري البحث...";
    list.innerHTML = '<div class="wifi-scanning">جاري البحث عن الشبكات...</div>';
    try {
        const response = await fetch("/scan", { method: "GET", cache: "no-store" });
        const data = await response.json();
        if (data.status === "scanning") {
            wifiScanTimer = setTimeout(pollWifiScan, 2000);
            return;
        }
        if (data.status === "complete" && data.networks) {
            renderWifiNetworks(data.networks);
        }
    } catch (error) {
        console.error("[WIFI] Scan error:", error);
        list.innerHTML = '<div class="wifi-scanning">فشل البحث</div>';
    } finally {
        btn.disabled = false;
        btn.textContent = "🔍 بحث عن شبكات";
    }
}

async function pollWifiScan() {
    try {
        const response = await fetch("/scan", { method: "GET", cache: "no-store" });
        const data = await response.json();
        if (data.status === "scanning") {
            wifiScanTimer = setTimeout(pollWifiScan, 1500);
            return;
        }
        if (data.status === "complete" && data.networks) {
            renderWifiNetworks(data.networks);
            const btn = $("wifiScanBtn");
            if (btn) { btn.disabled = false; btn.textContent = "🔍 بحث عن شبكات"; }
        }
    } catch (error) {
        console.error("[WIFI] Poll error:", error);
    }
}

function renderWifiNetworks(networks) {
    const list = $("wifiNetworkList");
    if (!list) return;
    if (networks.length === 0) {
        list.innerHTML = '<div class="wifi-scanning">لم يتم العثور على شبكات</div>';
        return;
    }
    networks.sort((a, b) => b.rssi - a.rssi);
    const currentSSID = $("wifiSSID").value;
    list.innerHTML = networks.map(net => {
        const signal = getSignalBars(net.rssi);
        const lock = net.encrypted ? "🔒" : "";
        const selected = net.ssid === currentSSID ? " selected" : "";
        return '<div class="wifi-network-item' + selected + '" data-ssid="' + escapeHtml(net.ssid) + '">' +
            '<span class="wifi-network-name">' + escapeHtml(net.ssid) + '</span>' +
            '<span class="wifi-network-info">' + lock + ' <span class="wifi-signal">' + signal + '</span> ' + net.rssi + ' dBm</span>' +
            '</div>';
    }).join("");
    list.querySelectorAll(".wifi-network-item").forEach(item => {
        item.addEventListener("click", () => {
            $("wifiSSID").value = item.dataset.ssid;
            list.querySelectorAll(".wifi-network-item").forEach(i => i.classList.remove("selected"));
            item.classList.add("selected");
        });
    });
}

function getSignalBars(rssi) {
    if (rssi >= -50) return "▂▄▆█";
    if (rssi >= -60) return "▂▄▆░";
    if (rssi >= -70) return "▂▄░░";
    return "▂░░░";
}

function escapeHtml(text) {
    const div = document.createElement("div");
    div.textContent = text;
    return div.innerHTML;
}


/* ==========================================================
   LOAD / SAVE NETWORK
   ========================================================== */

async function loadNetworkSettings() {
    try {
        const response = await fetch("/api/settings/network", { method: "GET", cache: "no-store" });
        if (!response.ok) throw new Error("HTTP " + response.status);
        const data = await response.json();
        $("wifiSSID").value = data.ssid ?? "";
        setSelect("wifiEnable", data.wifiEnable ?? true);
        $("mqttServer").value = data.mqttServer ?? "";
        setNumber("mqttPort", data.mqttPort ?? 1883);
        $("mqttUser").value = data.mqttUser ?? "";
        $("mqttTopic").value = data.mqttTopic ?? "prayer";
        setSelect("mqttEnable", data.mqttEnable ?? false);
    } catch (error) {
        console.error("[NETWORK] Load error:", error);
    }
}

async function saveNetworkSettings() {
    if (isSavingNetwork) return;
    isSavingNetwork = true;
    const btn = $("saveNetworkBtn");
    if (btn) { btn.disabled = true; btn.textContent = "⏳ جاري الحفظ..."; }
    try {
        const wifiPass = $("wifiPassword").value;
        const data = {
            wifiSSID: $("wifiSSID").value,
            mqttServer: $("mqttServer").value,
            mqttPort: clamp(readNumber("mqttPort", 1883), 1, 65535),
            mqttUser: $("mqttUser").value,
            mqttPassword: $("mqttPassword").value,
            mqttTopic: $("mqttTopic").value,
            mqttEnable: readBool("mqttEnable", false)
        };
        if (wifiPass.length > 0) {
            data.wifiPassword = wifiPass;
        }
        const response = await fetch("/api/settings/network", {
            method: "POST", cache: "no-store",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(data)
        });
        if (!response.ok) throw new Error("HTTP " + response.status);
        const result = await response.json();
        if (result.restart) {
            showToast("تم الحفظ — سيتم إعادة التشغيل...", "success");
        } else {
            showToast("تم حفظ إعدادات الشبكة بنجاح", "success");
        }
    } catch (error) {
        console.error("[NETWORK] Save error:", error);
        showToast("فشل حفظ إعدادات الشبكة", "error");
    } finally {
        isSavingNetwork = false;
        if (btn) { btn.disabled = false; btn.textContent = "💾 حفظ الشبكة"; }
    }
}


/* ==========================================================
   SYSTEM
   ========================================================== */

async function loadSystemInfo() {
    try {
        const response = await fetch("/api/system/info", { method: "GET", cache: "no-store" });
        if (!response.ok) throw new Error("HTTP " + response.status);
        const data = await response.json();
        setText("infoDevice", data.device ?? "ESP-Prayer-System");
        setText("infoVersion", data.version ?? "1.0.0");
        setText("infoWifiStatus", data.wifiConnected ? "متصل" : "غير متصل");
        const wifiEl = $("infoWifiStatus");
        if (wifiEl) wifiEl.className = data.wifiConnected ? "success" : "error";
        const durInput = $("eventDuration");
        if (durInput && data.eventDisplayDuration !== undefined) {
            durInput.value = data.eventDisplayDuration;
        }
    } catch (error) {
        console.error("[SYSTEM] Load error:", error);
    }
}

async function saveEventDuration() {
    const button = $("saveEventDurationBtn");
    const duration = parseInt($("eventDuration")?.value, 10);
    if (!Number.isFinite(duration)) {
        showToast("أدخل مدة صحيحة (2-60 ثانية)", "warning");
        return;
    }
    if (button) button.disabled = true;
    try {
        const response = await fetch("/api/settings/display", {
            method: "POST",
            cache: "no-store",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ eventDisplayDuration: Math.min(Math.max(duration, 2), 60) })
        });
        if (!response.ok) throw new Error("HTTP " + response.status);
        showToast("تم حفظ مدة العرض بنجاح", "success");
    } catch (error) {
        console.error("[DISPLAY] Save error:", error);
        showToast("فشل حفظ المدة", "error");
    } finally {
        if (button) button.disabled = false;
    }
}

async function restartDevice() {
    if (!confirm("هل أنت متأكد من إعادة تشغيل الجهاز؟")) return;
    try {
        await fetch("/api/system/restart", { method: "POST", cache: "no-store" });
        showToast("جاري إعادة التشغيل...", "warning");
    } catch (_) {}
}

async function resetSettings() {
    if (!confirm("⚠️ تحذير: سيتم مسح جميع الإعدادات والرجوع للإعدادات الافتراضية!")) return;
    if (!confirm("هل أنت متأكد؟ لن تتمكن من التراجع!")) return;
    const input = prompt('اكتب "مسح" للتأكيد:');
    if (input !== "مسح") { showToast("تم الإلغاء", "warning"); return; }
    try {
        await fetch("/api/system/reset", { method: "POST", cache: "no-store" });
        showToast("جاري مسح الإعدادات...", "warning");
    } catch (_) {}
}

async function exportSettings() {
    const btn = $("exportBtn");
    if (btn) { btn.disabled = true; btn.textContent = "⏳ جاري التحميل..."; }
    try {
        const r = await fetch("/api/settings/export", { cache: "no-store" });
        if (!r.ok) throw new Error("HTTP " + r.status);
        const blob = await r.blob();
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = "prayer-config.json";
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        showToast("تم تحميل الإعدادات", "success");
    } catch (e) {
        console.error("[EXPORT]", e);
        showToast("فشل تحميل الإعدادات", "error");
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = "📥 تحميل الإعدادات"; }
    }
}

function importSettings() {
    $("importFile")?.click();
}

async function importSettingsFile(file) {
    if (!file) return;
    if (!confirm("⚠️ سيتم استبدال جميع الإعدادات وإعادة تشغيل الجهاز!")) return;
    const btn = $("importBtn");
    if (btn) { btn.disabled = true; btn.textContent = "⏳ جاري الرفع..."; }
    try {
        const text = await file.text();
        JSON.parse(text); // validate
        const r = await fetch("/api/settings/import", {
            method: "POST", cache: "no-store",
            headers: { "Content-Type": "application/json" },
            body: text
        });
        if (!r.ok) throw new Error("HTTP " + r.status);
        showToast("تم رفع الإعدادات — جاري إعادة التشغيل...", "success");
    } catch (e) {
        console.error("[IMPORT]", e);
        showToast("فشل رفع الإعدادات — تأكد من صحة الملف", "error");
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = "📤 رفع الإعدادات"; }
    }
}


/* ==========================================================
   DHIKR COUNTER
   ========================================================== */

const dhikrState = {
    subhan: 0,
    hamd: 0,
    tawhid: 0
};

function loadDhikrState() {
    try {
        const saved = JSON.parse(localStorage.getItem("dhikr") || "{}");
        dhikrState.subhan = Number(saved.subhan) || 0;
        dhikrState.hamd = Number(saved.hamd) || 0;
        dhikrState.tawhid = Number(saved.tawhid) || 0;
    } catch (_) {}
    updateDhikrDisplay();
}

function saveDhikrState() {
    try { localStorage.setItem("dhikr", JSON.stringify(dhikrState)); } catch (_) {}
}

function updateDhikrDisplay() {
    for (const key of ["subhan", "hamd", "tawhid"]) {
        const el = $("dhikrCount_" + key);
        if (el) el.textContent = String(dhikrState[key]);
    }
}

async function tapDhikr(key) {
    const target = clamp(readNumber("dhikrTarget", 100), 1, 9999);
    dhikrState[key] = (dhikrState[key] + 1) % (target + 1);
    saveDhikrState();
    updateDhikrDisplay();
    try { await fetch("/api/test/click", { method: "POST", cache: "no-store" }); } catch (_) {}
}

function resetDhikr() {
    dhikrState.subhan = 0;
    dhikrState.hamd = 0;
    dhikrState.tawhid = 0;
    saveDhikrState();
    updateDhikrDisplay();
    showToast("تم إعادة العدّاد", "success");
}


/* ==========================================================
   RUQYAH PLAY / STOP
   ========================================================== */

async function playRuqyah() {
    const btn = $("ruqyahPlayBtn");
    const stopBtn = $("ruqyahStopBtn");
    if (btn) btn.disabled = true;
    try {
        const r = await fetch("/api/test?type=ruqyah", { method: "GET", cache: "no-store" });
        if (!r.ok) throw new Error("HTTP " + r.status);
        showToast("تم تشغيل الرقية", "success");
        if (stopBtn) stopBtn.style.display = "";
    } catch (e) {
        console.error("[RUQYAH]", e);
        showToast("فشل تشغيل الرقية", "error");
    } finally {
        if (btn) btn.disabled = false;
    }
}

async function stopRuqyah() {
    const stopBtn = $("ruqyahStopBtn");
    if (stopBtn) stopBtn.disabled = true;
    try {
        await fetch("/api/test?type=ruqyah-stop", { method: "GET", cache: "no-store" });
        showToast("تم إيقاف الرقية", "success");
        if (stopBtn) stopBtn.style.display = "none";
    } catch (e) {
        console.error("[RUQYAH-STOP]", e);
        showToast("فشل الإيقاف", "error");
    } finally {
        if (stopBtn) stopBtn.disabled = false;
    }
}


/* ==========================================================
   ADHKAR PLAYER
   ========================================================== */

async function playAdhkar() {
    const btn = $("adhkarPlayerPlayBtn");
    if (btn) btn.disabled = true;
    try {
        const file = clamp(readNumber("adhkarPlayerFile", 3), 1, 7);
        const volume = clamp(readNumber("adhkarPlayerVolume", 10), 0, 30);
        const r = await fetch(`/api/test?type=adhkar&file=${file}&volume=${volume}`, {
            method: "GET",
            cache: "no-store"
        });
        if (!r.ok) throw new Error("HTTP " + r.status);
        showToast("تم تشغيل الأذكار", "success");
    } catch (e) {
        console.error("[ADHKAR]", e);
        showToast("فشل تشغيل الأذكار", "error");
    } finally {
        if (btn) btn.disabled = false;
    }
}


/* ==========================================================
   ========================================================== */

function setupEvents() {

    /* Tabs */
    document.querySelectorAll(".tab-btn").forEach(btn => {
        btn.addEventListener("click", () => switchTab(btn.dataset.tab));
    });

    /* Volume range displays */
    document.querySelectorAll('input[type="range"]').forEach(input => {
        input.addEventListener("input", updateVolumeDisplays);
    });

    /* Quran type change */
    $("quranType")?.addEventListener("change", () => {
        readCurrentQuran();
        loadSelectedQuran();
    });

    /* Enable/disable visuals */
    [
        ["azanEnable", "azanCard"],
        ["iqamaEnable", "iqamaCard"],
        ["morningAdhkarEnable", "morningAdhkarCard"],
        ["eveningAdhkarEnable", "eveningAdhkarCard"],
        ["kahfEnable", "kahfCard"],
        ["quranEnable", "quranCard"],
        ["customAlertEnable", "customAlertCard"]
    ].forEach(([selectId, cardId]) => {
        $(selectId)?.addEventListener("change", () => updateEnabledVisual(selectId, cardId));
    });

    /* Device section toggles */
    $("azanDevice")?.addEventListener("change", updateDeviceSections);
    $("iqamaDevice")?.addEventListener("change", updateDeviceSections);

    /* Audio controls */
    $("audioPlayButton")?.addEventListener("click", audioPlay);
    $("audioPauseButton")?.addEventListener("click", audioPause);
    $("audioStopButton")?.addEventListener("click", audioStop);
    $("audioVolumeUpButton")?.addEventListener("click", audioVolumeUp);
    $("audioVolumeDownButton")?.addEventListener("click", audioVolumeDown);

    /* Test buttons */
    document.querySelectorAll("[data-test-endpoint]").forEach(button => {
        button.addEventListener("click", () => {
            testEndpoint(button.dataset.testEndpoint, button.dataset.testMessage, button);
        });
    });

    /* Quran test */
    $("quranTestButton")?.addEventListener("click", testQuran);

    /* Custom Alert: source toggle */
    $("customAlertSource")?.addEventListener("change", () => {
        updateCustomAlertSourceUI(readSelect("customAlertSource", 0));
    });

    /* Custom Alert: repeat toggle */
    $("customAlertRepeat")?.addEventListener("change", () => {
        updateCustomAlertRepeatUI(readSelect("customAlertRepeat", 0));
    });

    /* Folder play */
    $("folderButton")?.addEventListener("click", playFolder);
    $("folderStopButton")?.addEventListener("click", stopFolder);
    $("folderVolume")?.addEventListener("input", () => {
        const v = $("folderVolumeValue");
        if (v) v.textContent = String($("folderVolume")?.value || 0);
    });

    /* WiFi scan */
    $("wifiScanBtn")?.addEventListener("click", startWifiScan);

    /* Save buttons */
    $("savePrayerBtn")?.addEventListener("click", savePrayerTab);
    $("saveAlertBtn")?.addEventListener("click", saveAlertTab);
    $("saveAdhkarBtn")?.addEventListener("click", saveAdhkarTab);
    $("saveQuranBtn")?.addEventListener("click", saveQuranTab);
    $("saveNetworkBtn")?.addEventListener("click", saveNetworkSettings);

    /* System buttons */
    $("restartBtn")?.addEventListener("click", restartDevice);
    $("resetBtn")?.addEventListener("click", resetSettings);
    $("exportBtn")?.addEventListener("click", exportSettings);
    $("importBtn")?.addEventListener("click", importSettings);
    $("importFile")?.addEventListener("change", (e) => {
        importSettingsFile(e.target.files[0]);
        e.target.value = "";
    });

    /* Dhikr counter */
    document.querySelectorAll("[data-dhikr]").forEach(el => {
        el.addEventListener("click", () => tapDhikr(el.dataset.dhikr));
    });
    $("dhikrResetBtn")?.addEventListener("click", resetDhikr);
    loadDhikrState();

    /* Adhkar player */
    $("adhkarPlayerPlayBtn")?.addEventListener("click", playAdhkar);
    $("adhkarPlayerVolume")?.addEventListener("input", () => {
        const v = $("adhkarPlayerVolumeValue");
        if (v) v.textContent = String($("adhkarPlayerVolume")?.value || 0);
    });

    /* Ruqyah */
    $("ruqyahPlayBtn")?.addEventListener("click", playRuqyah);
    $("ruqyahStopBtn")?.addEventListener("click", stopRuqyah);

    /* Volume labels */
    $("eidTakbeeratVolume")?.addEventListener("input", () => {
        const v = $("eidTakbeeratVolumeValue");
        if (v) v.textContent = String($("eidTakbeeratVolume")?.value || 0);
    });
    $("ruqyahVolume")?.addEventListener("input", () => {
        const v = $("ruqyahVolumeValue");
        if (v) v.textContent = String($("ruqyahVolume")?.value || 0);
    });
    $("dhikrRepeatVolume")?.addEventListener("input", () => {
        const v = $("dhikrRepeatVolumeValue");
        if (v) v.textContent = String($("dhikrRepeatVolume")?.value || 0);
    });

    /* Event duration save */
    $("saveEventDurationBtn")?.addEventListener("click", saveEventDuration);

    /* Back button */
    $("backButton")?.addEventListener("click", () => { window.location.href = "index.html"; });
}


/* ==========================================================
   AUTO-DETECT LOCATION
   ========================================================== */

async function detectLocation() {
    const btn = $("detectLocationBtn");
    if (btn) { btn.disabled = true; btn.textContent = "⏳ جاري كشف الموقع..."; }
    try {
        const response = await fetch("http://ip-api.com/json/?fields=status,country,city,lat,lon");
        const data = await response.json();
        if (data.status === "success") {
            $("city").value = data.city ?? "";
            $("country").value = data.country ?? "";
            $("latitude").value = data.lat ?? "";
            $("longitude").value = data.lon ?? "";
            showToast("تم كشف الموقع: " + (data.city ?? "") + ", " + (data.country ?? ""), "success");
        } else {
            showToast("لم يتم العثور على الموقع", "warning");
        }
    } catch (error) {
        console.error("[LOCATION]", error);
        showToast("فشل كشف الموقع — تأكد من الاتصال بالإنترنت", "error");
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = "📍 كشف الموقع تلقائياً"; }
    }
}


/* ==========================================================
   STARTUP
   ========================================================== */

document.addEventListener("DOMContentLoaded", async () => {
    setupEvents();
    updateVolumeDisplays();
    loadSelectedQuran();

    isSettingsPage = !!$("settingsApp") || !!$("city");

    if (isSettingsPage) {
        await Promise.all([
            loadAudioSettings(),
            loadPrayerSettings(),
            loadNetworkSettings(),
            loadSystemInfo()
        ]);

        initTabs();
        initLogFilters();

        if (location.hash === "#logs") {
            var logsBtn = document.querySelector('[data-tab="logs"]');
            if (logsBtn) logsBtn.click();
        }
    }
});


/* ==========================================================
   EVENT LOGS
   ========================================================== */

var currentLogFilter = "all";

function initLogFilters()
{
    document.querySelectorAll(".log-filter-btn").forEach(function(btn)
    {
        btn.addEventListener("click", function()
        {
            document.querySelectorAll(".log-filter-btn").forEach(function(b)
            {
                b.classList.remove("active");
            });

            btn.classList.add("active");

            currentLogFilter = btn.getAttribute("data-filter");

            loadLogs();
        });
    });
}

function loadLogs()
{
    fetch("/api/logs")
    .then(function(r) { return r.json(); })
    .then(function(data)
    {
        var container = document.getElementById("logContainer");

        if (!data || data.length === 0)
        {
            container.innerHTML = '<div class="log-empty">لا توجد سجلات</div>';
            return;
        }

        var filtered = data;

        if (currentLogFilter !== "all")
        {
            filtered = data.filter(function(e)
            {
                return e.cat === currentLogFilter;
            });
        }

        if (filtered.length === 0)
        {
            container.innerHTML = '<div class="log-empty">لا توجد سجلات في هذا القسم</div>';
            return;
        }

        var html = '<table class="log-table">';
        html += '<thead><tr>';
        html += '<th>الوقت</th>';
        html += '<th>الفئة</th>';
        html += '<th>الحدث</th>';
        html += '<th>المصدر</th>';
        html += '<th>الحالة</th>';
        html += '<th>التفاصيل</th>';
        html += '</tr></thead><tbody>';

        for (var i = 0; i < filtered.length; i++)
        {
            var e = filtered[i];
            var t = new Date(e.ts * 1000);

            var timeStr = t.getFullYear() + "-" +
                String(t.getMonth()+1).padStart(2,"0") + "-" +
                String(t.getDate()).padStart(2,"0") + " " +
                String(t.getHours()).padStart(2,"0") + ":" +
                String(t.getMinutes()).padStart(2,"0") + ":" +
                String(t.getSeconds()).padStart(2,"0");

            var statusClass = "log-status-" + e.status;
            var catClass = "log-cat-" + e.cat.toLowerCase();

            html += '<tr>';
            html += '<td>' + timeStr + '</td>';
            html += '<td class="' + catClass + '">' + e.cat + '</td>';
            html += '<td>' + e.action + '</td>';
            html += '<td>' + e.src + '</td>';
            html += '<td class="' + statusClass + '">' + e.status + '</td>';
            html += '<td>' + (e.detail || "") + '</td>';
            html += '</tr>';
        }

        html += '</tbody></table>';

        container.innerHTML = html;
    })
    .catch(function()
    {
        var container = document.getElementById("logContainer");
        container.innerHTML = '<div class="log-empty">خطأ في تحميل السجلات</div>';
    });
}

function clearLogs()
{
    if (!confirm("هل أنت متأكد من مسح كل السجلات؟"))
        return;

    fetch("/api/logs", { method: "POST" })
    .then(function(r) { return r.json(); })
    .then(function()
    {
        loadLogs();
    })
    .catch(function() {});
}