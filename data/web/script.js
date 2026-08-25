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

                weekday:
                    "long",

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
            4000
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


// =====================================
// Sync ESP Status Every 2 Seconds
// =====================================

setInterval(
    loadStatus,
    2000
);