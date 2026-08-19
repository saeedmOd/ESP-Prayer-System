/* =========================================================
   ESP PRAYER SYSTEM
   audio.js
   =========================================================

   مسؤول عن:

   - إعدادات الصوت
   - الأذان
   - الإقامة
   - أذكار الصباح
   - أذكار المساء
   - سورة الكهف
   - القرآن
   - التحكم بالتشغيل
   - Volume
   - اختبار الأصوات
   - تحميل / حفظ إعدادات الصوت

   API:

   GET  /api/settings/audio
   POST /api/settings/audio

   POST /api/settings/volume

   POST /api/test/azan
   POST /api/test/iqama
   POST /api/test/morning-adhkar
   POST /api/test/evening-adhkar
   POST /api/test/kahf
   POST /api/test/quran
   POST /api/test/folder

   POST /api/audio/play
   POST /api/audio/pause
   POST /api/audio/stop
   POST /api/audio/volume-up
   POST /api/audio/volume-down
   ========================================================= */


/* =========================================================
   DEFAULTS
   ========================================================= */

const AUDIO_DEFAULTS = {

    volume: 1,

    azanEnable: true,
    azanFolder: 1,
    azanFile: 1,

    iqamaEnable: false,
    iqamaFolder: 1,
    iqamaFile: 1,
    iqamaVolume: 1,
    iqamaDelay: 10,

    iqamaFajr: false,
    iqamaDhuhr: false,
    iqamaAsr: false,
    iqamaMaghrib: false,
    iqamaIsha: false,

    morningAdhkarEnable: false,
    morningAdhkarHour: 6,
    morningAdhkarMinute: 0,
    morningAdhkarVolume: 1,
    morningAdhkarFolder: 4,
    morningAdhkarFile: 1,

    eveningAdhkarEnable: false,
    eveningAdhkarHour: 18,
    eveningAdhkarMinute: 0,
    eveningAdhkarVolume: 1,
    eveningAdhkarFolder: 4,
    eveningAdhkarFile: 2,

    kahfEnable: false,
    kahfHour: 9,
    kahfMinute: 0,
    kahfVolume: 1,
    kahfFolder: 2,
    kahfFile: 1,

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
        volume: 15
    }
};


/* =========================================================
   CONSTANTS
   ========================================================= */

const IQAMA_FOLDER = 1;
const IQAMA_FILE = 1;


/* =========================================================
   QURAN LIST
   ========================================================= */

const QURAN_LIST = {

    baqarah: {
        name: "البقرة",
        folder: 2,
        file: 1
    },

    baqarahLast: {
        name: "آخر البقرة",
        folder: 2,
        file: 2
    },

    ayatKursi: {
        name: "آية الكرسي",
        folder: 2,
        file: 3
    },

    maryam: {
        name: "مريم",
        folder: 2,
        file: 4
    }
};


/* =========================================================
   QURAN DEFAULTS
   ========================================================= */

const DEFAULT_QURAN_SETTINGS = {

    enable: false,

    hour: 0,

    minute: 0,

    volume: 1,

    selected: "baqarah"
};


/* =========================================================
   CURRENT QURAN SETTINGS
   ========================================================= */

let quranSettings = {
    ...DEFAULT_QURAN_SETTINGS
};


/* =========================================================
   SAVE LOCK
   ========================================================= */

let audioSaveInProgress = false;


/* =========================================================
   DOM HELPERS
   ========================================================= */

function getElement(id) {

    return document.getElementById(id);
}


function showValue(id, value) {

    const element = getElement(id);

    if (!element) {
        return;
    }

    element.textContent =
        value ?? "";
}


function setNumber(id, value) {

    const element = getElement(id);

    if (!element) {
        return;
    }

    element.value =
        value ?? "";
}


function setSelect(id, value) {

    const element = getElement(id);

    if (!element) {
        return;
    }

    if (typeof value === "boolean") {

        element.value =
            value ? "true" : "false";

        return;
    }

    element.value =
        String(value ?? "");
}


function setCheckbox(id, value) {

    const element = getElement(id);

    if (!element) {
        return;
    }

    element.checked =
        value === true;
}


/* =========================================================
   READ HELPERS
   ========================================================= */

function readNumber(id, fallback = 0) {

    const element = getElement(id);

    if (!element) {
        return fallback;
    }

    const value =
        Number(element.value);

    return Number.isFinite(value)
        ? value
        : fallback;
}


function readBool(id, fallback = false) {

    const element = getElement(id);

    if (!element) {
        return fallback;
    }

    if (element.type === "checkbox") {
        return element.checked;
    }

    if (element.value === "true") {
        return true;
    }

    if (element.value === "false") {
        return false;
    }

    return fallback;
}


function readCheckbox(id, fallback = false) {

    const element = getElement(id);

    if (!element) {
        return fallback;
    }

    return Boolean(
        element.checked
    );
}


/* =========================================================
   PRESET HELPERS
   ========================================================= */

function splitPreset(value) {

    const parts =
        String(value || "")
            .split(":");

    return {

        folder:
            Number(parts[0]) || 0,

        file:
            Number(parts[1]) || 0
    };
}


function setPresetFromValues(
    type,
    folder,
    file
) {

    const preset =
        getElement(
            type + "Preset"
        );

    if (!preset) {
        return;
    }

    const target =
        Number(folder) +
        ":" +
        Number(file);

    const option =
        Array.from(
            preset.options
        ).find(
            item =>
                item.value === target
        );

    if (option) {
        preset.value = target;
    }
}


/* =========================================================
   PRESET ACTIONS
   ========================================================= */

function applyPreset(type) {

    const preset =
        getElement(
            type + "Preset"
        );

    if (!preset) {
        return;
    }

    const selected =
        splitPreset(
            preset.value
        );

    console.log(
        "[AUDIO] Preset:",
        type,
        selected
    );
}


function applyIqamaPreset() {

    applyPreset("iqama");
}


/* =========================================================
   QURAN
   ========================================================= */

function getSelectedQuran() {

    const select =
        getElement(
            "quranType"
        );

    if (!select) {
        return null;
    }

    return (
        select.value ||
        DEFAULT_QURAN_SETTINGS.selected
    );
}


function getSelectedQuranFile() {

    const select =
        getElement(
            "quranType"
        );

    if (!select) {
        return null;
    }

    const option =
        select.options[
            select.selectedIndex
        ];

    if (!option) {
        return null;
    }

    const type =
        option.value;

    const config =
        QURAN_LIST[type];

    if (!config) {

        console.warn(
            "[QURAN] Unknown selection:",
            type
        );

        return null;
    }

    let folder =
        Number(
            option.dataset.folder
        );

    let file =
        Number(
            option.dataset.file
        );

    if (
        !Number.isFinite(folder) ||
        folder <= 0
    ) {

        folder =
            config.folder;
    }

    if (
        !Number.isFinite(file) ||
        file <= 0
    ) {

        file =
            config.file;
    }

    return {

        type: type,

        name:
            config.name,

        folder:
            folder,

        file:
            file
    };
}


/* =========================================================
   LOAD SELECTED QURAN
   ========================================================= */

function loadSelectedQuran() {

    const select =
        getElement(
            "quranType"
        );

    if (!select) {

        console.warn(
            "[QURAN] quranType not found"
        );

        return;
    }

    let selected =
        quranSettings.selected;

    if (!QURAN_LIST[selected]) {

        selected =
            DEFAULT_QURAN_SETTINGS.selected;
    }

    select.value =
        selected;

    setSelect(
        "quranEnable",
        quranSettings.enable
    );

    setNumber(
        "quranHour",
        quranSettings.hour
    );

    setNumber(
        "quranMinute",
        quranSettings.minute
    );

    setNumber(
        "quranVolume",
        quranSettings.volume
    );

    showValue(
        "quranVolumeValue",
        quranSettings.volume
    );

    console.log(
        "[QURAN] Loaded:",
        quranSettings
    );
}


/* =========================================================
   READ CURRENT QURAN
   ========================================================= */

function readCurrentQuran() {

    const selected =
        getSelectedQuranFile();

    if (!selected) {

        console.warn(
            "[QURAN] No Quran selected"
        );

        return false;
    }

    quranSettings = {

        enable:
            readBool(
                "quranEnable",
                DEFAULT_QURAN_SETTINGS.enable
            ),

        hour:
            readNumber(
                "quranHour",
                DEFAULT_QURAN_SETTINGS.hour
            ),

        minute:
            readNumber(
                "quranMinute",
                DEFAULT_QURAN_SETTINGS.minute
            ),

        volume:
            readNumber(
                "quranVolume",
                DEFAULT_QURAN_SETTINGS.volume
            ),

        selected:
            selected.type
    };

    console.log(
        "[QURAN] Current:",
        quranSettings
    );

    return true;
}


/* =========================================================
   GENERAL VOLUME
   ========================================================= */

function showVolume(value) {

    showValue(
        "volumeValue",
        value
    );
}


/* =========================================================
   SAVE VOLUME
   ========================================================= */

async function saveVolume(value) {

    const volume =
        Number(value);

    if (!Number.isFinite(volume)) {
        return false;
    }

    showVolume(volume);

    setNumber(
        "volume",
        volume
    );

    try {

        const response =
            await fetch(
                "/api/settings/volume",
                {

                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            volume: volume
                        })
                }
            );

        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );
        }

        console.log(
            "[VOLUME] Saved"
        );

        return true;

    }
    catch (error) {

        console.error(
            "[VOLUME]",
            error
        );

        return false;
    }
}


/* =========================================================
   AUDIO REQUEST
   ========================================================= */

async function audioRequest(
    endpoint,
    options = {}
) {

    console.log(
        "[AUDIO]",
        endpoint
    );

    try {

        const response =
            await fetch(
                endpoint,
                {

                    method:
                        options.method ||
                        "POST",

                    headers:
                        options.headers ||
                        {},

                    body:
                        options.body
                }
            );

        const contentType =
            response.headers.get(
                "content-type"
            );

        let result;

        if (
            contentType &&
            contentType.includes(
                "application/json"
            )
        ) {

            result =
                await response.json();

        }
        else {

            result =
                await response.text();
        }

        console.log(
            "[AUDIO] HTTP:",
            response.status
        );

        console.log(
            "[AUDIO] Response:",
            result
        );

        if (!response.ok) {

            const message =
                result &&
                typeof result === "object"
                    ? result.message
                    : null;

            throw new Error(
                message ||
                "HTTP " +
                response.status
            );
        }

        return result;

    }
    catch (error) {

        console.error(
            "[AUDIO] Request error:",
            error
        );

        throw error;
    }
}


/* =========================================================
   PLAYBACK
   ========================================================= */

function audioPlay() {

    return audioRequest(
        "/api/audio/play"
    );
}


function audioPause() {

    return audioRequest(
        "/api/audio/pause"
    );
}


function audioStop() {

    return audioRequest(
        "/api/audio/stop"
    );
}


function audioVolumeUp() {

    return audioRequest(
        "/api/audio/volume-up"
    );
}


function audioVolumeDown() {

    return audioRequest(
        "/api/audio/volume-down"
    );
}


/* =========================================================
   TEST REQUEST
   ========================================================= */

async function testEndpoint(
    endpoint,
    successMessage
) {

    try {

        const result =
            await audioRequest(
                endpoint
            );

        console.log(
            "[TEST]",
            endpoint,
            result
        );

        alert(
            successMessage
        );

        return true;

    }
    catch (error) {

        console.error(
            "[TEST]",
            error
        );

        alert(
            "فشل تشغيل الصوت: " +
            error.message
        );

        return false;
    }
}


/* =========================================================
   TESTS
   ========================================================= */

function testAzan() {

    return testEndpoint(
        "/api/test/azan",
        "تم تشغيل الأذان"
    );
}


function testIqama() {

    return testEndpoint(
        "/api/test/iqama",
        "تم تشغيل الإقامة"
    );
}


function testMorningAdhkar() {

    return testEndpoint(
        "/api/test/morning-adhkar",
        "تم تشغيل أذكار الصباح"
    );
}


function testEveningAdhkar() {

    return testEndpoint(
        "/api/test/evening-adhkar",
        "تم تشغيل أذكار المساء"
    );
}


function testKahf() {

    return testEndpoint(
        "/api/test/kahf",
        "تم تشغيل سورة الكهف"
    );
}


/* =========================================================
   TEST QURAN
   ========================================================= */

async function testQuran() {

    if (!readCurrentQuran()) {

        alert(
            "لم يتم اختيار محتوى القرآن"
        );

        return false;
    }

    const selected =
        getSelectedQuranFile();

    if (!selected) {

        alert(
            "لم يتم اختيار محتوى القرآن"
        );

        return false;
    }

    try {

        const result =
            await audioRequest(
                "/api/test/quran",
                {

                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({

                            type:
                                selected.type,

                            folder:
                                selected.folder,

                            file:
                                selected.file,

                            volume:
                                quranSettings.volume
                        })
                }
            );

        console.log(
            "[QURAN TEST]",
            result
        );

        alert(
            "تم تشغيل المحتوى المختار"
        );

        return true;

    }
    catch (error) {

        console.error(
            "[QURAN TEST]",
            error
        );

        alert(
            "فشل تشغيل المحتوى: " +
            error.message
        );

        return false;
    }
}


/* =========================================================
   PLAY FOLDER
   ========================================================= */

async function playFolder() {

    const folder =
        readNumber(
            "folder",
            1
        );

    const volume =
        readNumber(
            "folderVolume",
            1
        );

    try {

        const result =
            await audioRequest(
                "/api/test/folder",
                {

                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({

                            folder:
                                folder,

                            volume:
                                volume
                        })
                }
            );

        console.log(
            "[FOLDER TEST]",
            result
        );

        alert(
            "تم تشغيل المجلد"
        );

        return true;

    }
    catch (error) {

        console.error(
            "[FOLDER TEST]",
            error
        );

        alert(
            "فشل تشغيل المجلد: " +
            error.message
        );

        return false;
    }
}


/* =========================================================
   BUILD AUDIO PAYLOAD
   ========================================================= */

function buildAudioPayload() {

    readCurrentQuran();

    /* -----------------------------------------------------
       AZAN
       ----------------------------------------------------- */

    let azanFolder =
        AUDIO_DEFAULTS.azanFolder;

    let azanFile =
        AUDIO_DEFAULTS.azanFile;

    const azanPreset =
        getElement(
            "azanPreset"
        );

    if (azanPreset) {

        const selected =
            splitPreset(
                azanPreset.value
            );

        if (selected.folder > 0) {
            azanFolder =
                selected.folder;
        }

        if (selected.file > 0) {
            azanFile =
                selected.file;
        }
    }


    /* -----------------------------------------------------
       IQAMA
       ----------------------------------------------------- */

    let iqamaFolder =
        IQAMA_FOLDER;

    let iqamaFile =
        IQAMA_FILE;

    const iqamaPreset =
        getElement(
            "iqamaPreset"
        );

    if (iqamaPreset) {

        const selected =
            splitPreset(
                iqamaPreset.value
            );

        if (selected.folder > 0) {
            iqamaFolder =
                selected.folder;
        }

        if (selected.file > 0) {
            iqamaFile =
                selected.file;
        }
    }


    /* -----------------------------------------------------
       PAYLOAD
       ----------------------------------------------------- */

    return {

        volume:
            readNumber(
                "volume",
                AUDIO_DEFAULTS.volume
            ),


        azanEnable:
            readBool(
                "azanEnable",
                AUDIO_DEFAULTS.azanEnable
            ),

        azanFolder:
            azanFolder,

        azanFile:
            azanFile,


        iqamaEnable:
            readBool(
                "iqamaEnable",
                AUDIO_DEFAULTS.iqamaEnable
            ),

        iqamaFolder:
            iqamaFolder,

        iqamaFile:
            iqamaFile,

        iqamaVolume:
            readNumber(
                "iqamaVolume",
                AUDIO_DEFAULTS.iqamaVolume
            ),

        iqamaDelay:
            readNumber(
                "iqamaDelay",
                AUDIO_DEFAULTS.iqamaDelay
            ),


        iqamaFajr:
            readCheckbox(
                "iqamaFajr",
                AUDIO_DEFAULTS.iqamaFajr
            ),

        iqamaDhuhr:
            readCheckbox(
                "iqamaDhuhr",
                AUDIO_DEFAULTS.iqamaDhuhr
            ),

        iqamaAsr:
            readCheckbox(
                "iqamaAsr",
                AUDIO_DEFAULTS.iqamaAsr
            ),

        iqamaMaghrib:
            readCheckbox(
                "iqamaMaghrib",
                AUDIO_DEFAULTS.iqamaMaghrib
            ),

        iqamaIsha:
            readCheckbox(
                "iqamaIsha",
                AUDIO_DEFAULTS.iqamaIsha
            ),


        morningAdhkarEnable:
            readBool(
                "morningAdhkarEnable",
                AUDIO_DEFAULTS.morningAdhkarEnable
            ),

        morningAdhkarHour:
            readNumber(
                "morningAdhkarHour",
                AUDIO_DEFAULTS.morningAdhkarHour
            ),

        morningAdhkarMinute:
            readNumber(
                "morningAdhkarMinute",
                AUDIO_DEFAULTS.morningAdhkarMinute
            ),

        morningAdhkarVolume:
            readNumber(
                "morningAdhkarVolume",
                AUDIO_DEFAULTS.morningAdhkarVolume
            ),


        eveningAdhkarEnable:
            readBool(
                "eveningAdhkarEnable",
                AUDIO_DEFAULTS.eveningAdhkarEnable
            ),

        eveningAdhkarHour:
            readNumber(
                "eveningAdhkarHour",
                AUDIO_DEFAULTS.eveningAdhkarHour
            ),

        eveningAdhkarMinute:
            readNumber(
                "eveningAdhkarMinute",
                AUDIO_DEFAULTS.eveningAdhkarMinute
            ),

        eveningAdhkarVolume:
            readNumber(
                "eveningAdhkarVolume",
                AUDIO_DEFAULTS.eveningAdhkarVolume
            ),


        kahfEnable:
            readBool(
                "kahfEnable",
                AUDIO_DEFAULTS.kahfEnable
            ),

        kahfHour:
            readNumber(
                "kahfHour",
                AUDIO_DEFAULTS.kahfHour
            ),

        kahfMinute:
            readNumber(
                "kahfMinute",
                AUDIO_DEFAULTS.kahfMinute
            ),

        kahfVolume:
            readNumber(
                "kahfVolume",
                AUDIO_DEFAULTS.kahfVolume
            ),


        alarmToneType:
            readNumber(
                "alarmToneType",
                AUDIO_DEFAULTS.alarmToneType
            ),


        customAlertEnable:
            readBool(
                "customAlertEnable",
                AUDIO_DEFAULTS.customAlert.enable
            ),

        customAlertSource:
            readNumber(
                "customAlertSource",
                AUDIO_DEFAULTS.customAlert.source
            ),

        customAlertHour:
            readNumber(
                "customAlertHour",
                AUDIO_DEFAULTS.customAlert.hour
            ),

        customAlertMinute:
            readNumber(
                "customAlertMinute",
                AUDIO_DEFAULTS.customAlert.minute
            ),

        customAlertDays:
            readNumber(
                "customAlertDays",
                AUDIO_DEFAULTS.customAlert.days
            ),

        customAlertRepeat:
            readNumber(
                "customAlertRepeat",
                AUDIO_DEFAULTS.customAlert.repeat
            ),

        customAlertInterval:
            readNumber(
                "customAlertInterval",
                AUDIO_DEFAULTS.customAlert.interval
            ),

        customAlertFile:
            readNumber(
                "customAlertFile",
                AUDIO_DEFAULTS.customAlert.file
            ),

        customAlertVolume:
            readNumber(
                "customAlertVolume",
                AUDIO_DEFAULTS.customAlert.volume
            ),


        quran: {
            ...quranSettings
        }
    };
}


/* =========================================================
   LOAD AUDIO SETTINGS
   ========================================================= */

async function loadAudioSettings() {

    console.log(
        "[AUDIO] Loading settings..."
    );

    const status =
        getElement("status");

    try {

        const response =
            await fetch(
                "/api/settings/audio",
                {
                    method: "GET",
                    cache: "no-store"
                }
            );

        if (!response.ok) {

            throw new Error(
                "HTTP " +
                response.status
            );
        }

        const data =
            await response.json();

        console.log(
            "[AUDIO] Loaded:",
            data
        );


        /* =================================================
           GENERAL
           ================================================= */

        const volume =
            data.volume ??
            AUDIO_DEFAULTS.volume;

        setNumber(
            "volume",
            volume
        );

        showVolume(volume);


        /* =================================================
           AZAN
           ================================================= */

        setSelect(
            "azanEnable",
            data.azanEnable ??
            AUDIO_DEFAULTS.azanEnable
        );

        if (
            data.azanFolder !== undefined &&
            data.azanFile !== undefined
        ) {

            setPresetFromValues(
                "azan",
                data.azanFolder,
                data.azanFile
            );
        }


        /* =================================================
           IQAMA
           ================================================= */

        setSelect(
            "iqamaEnable",
            data.iqamaEnable ??
            AUDIO_DEFAULTS.iqamaEnable
        );

        setNumber(
            "iqamaVolume",
            data.iqamaVolume ??
            AUDIO_DEFAULTS.iqamaVolume
        );

        showValue(
            "iqamaVolumeValue",
            data.iqamaVolume ??
            AUDIO_DEFAULTS.iqamaVolume
        );

        setNumber(
            "iqamaDelay",
            data.iqamaDelay ??
            AUDIO_DEFAULTS.iqamaDelay
        );

        setCheckbox(
            "iqamaFajr",
            data.iqamaFajr ??
            AUDIO_DEFAULTS.iqamaFajr
        );

        setCheckbox(
            "iqamaDhuhr",
            data.iqamaDhuhr ??
            AUDIO_DEFAULTS.iqamaDhuhr
        );

        setCheckbox(
            "iqamaAsr",
            data.iqamaAsr ??
            AUDIO_DEFAULTS.iqamaAsr
        );

        setCheckbox(
            "iqamaMaghrib",
            data.iqamaMaghrib ??
            AUDIO_DEFAULTS.iqamaMaghrib
        );

        setCheckbox(
            "iqamaIsha",
            data.iqamaIsha ??
            AUDIO_DEFAULTS.iqamaIsha
        );

        if (
            data.iqamaFolder !== undefined &&
            data.iqamaFile !== undefined
        ) {

            setPresetFromValues(
                "iqama",
                data.iqamaFolder,
                data.iqamaFile
            );
        }


        /* =================================================
           MORNING ADHKAR
           ================================================= */

        setSelect(
            "morningAdhkarEnable",
            data.morningAdhkarEnable ??
            AUDIO_DEFAULTS.morningAdhkarEnable
        );

        setNumber(
            "morningAdhkarHour",
            data.morningAdhkarHour ??
            AUDIO_DEFAULTS.morningAdhkarHour
        );

        setNumber(
            "morningAdhkarMinute",
            data.morningAdhkarMinute ??
            AUDIO_DEFAULTS.morningAdhkarMinute
        );

        setNumber(
            "morningAdhkarVolume",
            data.morningAdhkarVolume ??
            AUDIO_DEFAULTS.morningAdhkarVolume
        );

        showValue(
            "morningAdhkarVolumeValue",
            data.morningAdhkarVolume ??
            AUDIO_DEFAULTS.morningAdhkarVolume
        );


        /* =================================================
           EVENING ADHKAR
           ================================================= */

        setSelect(
            "eveningAdhkarEnable",
            data.eveningAdhkarEnable ??
            AUDIO_DEFAULTS.eveningAdhkarEnable
        );

        setNumber(
            "eveningAdhkarHour",
            data.eveningAdhkarHour ??
            AUDIO_DEFAULTS.eveningAdhkarHour
        );

        setNumber(
            "eveningAdhkarMinute",
            data.eveningAdhkarMinute ??
            AUDIO_DEFAULTS.eveningAdhkarMinute
        );

        setNumber(
            "eveningAdhkarVolume",
            data.eveningAdhkarVolume ??
            AUDIO_DEFAULTS.eveningAdhkarVolume
        );

        showValue(
            "eveningAdhkarVolumeValue",
            data.eveningAdhkarVolume ??
            AUDIO_DEFAULTS.eveningAdhkarVolume
        );


        /* =================================================
           KAHF
           ================================================= */

        setSelect(
            "kahfEnable",
            data.kahfEnable ??
            AUDIO_DEFAULTS.kahfEnable
        );

        setNumber(
            "kahfHour",
            data.kahfHour ??
            AUDIO_DEFAULTS.kahfHour
        );

        setNumber(
            "kahfMinute",
            data.kahfMinute ??
            AUDIO_DEFAULTS.kahfMinute
        );

        setNumber(
            "kahfVolume",
            data.kahfVolume ??
            AUDIO_DEFAULTS.kahfVolume
        );

        showValue(
            "kahfVolumeValue",
            data.kahfVolume ??
            AUDIO_DEFAULTS.kahfVolume
        );


        /* =================================================
           ALARM TONE TYPE
           ================================================= */

        setNumber(
            "alarmToneType",
            data.alarmToneType ??
            AUDIO_DEFAULTS.alarmToneType
        );


        /* =================================================
           CUSTOM ALERT
           ================================================= */

        if (data.customAlertEnable !== undefined) {

            setSelect(
                "customAlertEnable",
                data.customAlertEnable ??
                AUDIO_DEFAULTS.customAlert.enable
            );

        }

        if (data.customAlertSource !== undefined) {

            setSelect(
                "customAlertSource",
                data.customAlertSource ??
                AUDIO_DEFAULTS.customAlert.source
            );

        }

        if (data.customAlertHour !== undefined) {

            setNumber(
                "customAlertHour",
                data.customAlertHour ??
                AUDIO_DEFAULTS.customAlert.hour
            );

        }

        if (data.customAlertMinute !== undefined) {

            setNumber(
                "customAlertMinute",
                data.customAlertMinute ??
                AUDIO_DEFAULTS.customAlert.minute
            );

        }

        if (data.customAlertRepeat !== undefined) {

            setSelect(
                "customAlertRepeat",
                data.customAlertRepeat ??
                AUDIO_DEFAULTS.customAlert.repeat
            );

        }

        if (data.customAlertInterval !== undefined) {

            setNumber(
                "customAlertInterval",
                data.customAlertInterval ??
                AUDIO_DEFAULTS.customAlert.interval
            );

        }

        if (data.customAlertFile !== undefined) {

            setSelect(
                "customAlertFile",
                data.customAlertFile ??
                AUDIO_DEFAULTS.customAlert.file
            );

        }

        if (data.customAlertVolume !== undefined) {

            setNumber(
                "customAlertVolume",
                data.customAlertVolume ??
                AUDIO_DEFAULTS.customAlert.volume
            );

        }


        /* =================================================
           QURAN
           ================================================= */

        if (
            data.quran &&
            typeof data.quran === "object"
        ) {

            quranSettings = {

                ...DEFAULT_QURAN_SETTINGS,

                ...data.quran
            };

        }
        else {

            quranSettings = {

                ...DEFAULT_QURAN_SETTINGS
            };
        }


        if (
            !QURAN_LIST[
                quranSettings.selected
            ]
        ) {

            quranSettings.selected =
                DEFAULT_QURAN_SETTINGS.selected;
        }


        loadSelectedQuran();


        /* =================================================
           STATUS
           ================================================= */

        if (status) {

            status.textContent =
                "متصل";
        }

        console.log(
            "[AUDIO] Settings loaded successfully"
        );

    }
    catch (error) {

        console.error(
            "[AUDIO] Load error:",
            error
        );

        if (status) {

            status.textContent =
                "خطأ اتصال";
        }
    }
}


/* =========================================================
   SAVE AUDIO SETTINGS
   ========================================================= */

async function saveAudioSettings() {

    if (audioSaveInProgress) {

        console.warn(
            "[AUDIO] Save already running"
        );

        return false;
    }

    audioSaveInProgress = true;

    const status =
        getElement("status");

    try {

        const data =
            buildAudioPayload();

        console.log(
            "[AUDIO] Saving:"
        );

        console.log(
            JSON.stringify(
                data,
                null,
                2
            )
        );

        if (status) {

            status.textContent =
                "جارٍ الحفظ...";
        }


        const response =
            await fetch(
                "/api/settings/audio",
                {

                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(data)
                }
            );


        if (!response.ok) {

            const text =
                await response.text();

            throw new Error(
                "HTTP " +
                response.status +
                " - " +
                text
            );
        }


        const result =
            await response.text();

        console.log(
            "[AUDIO] Save response:",
            result
        );


        /* -------------------------------------------------
           بعد نجاح الحفظ نحدث الحالة المحلية
           ------------------------------------------------- */

        quranSettings =
            data.quran || {
                ...DEFAULT_QURAN_SETTINGS
            };


        if (status) {

            status.textContent =
                "تم الحفظ";
        }


        alert(
            "تم حفظ إعدادات الصوت بنجاح"
        );

        return true;

    }
    catch (error) {

        console.error(
            "[AUDIO] Save error:",
            error
        );

        if (status) {

            status.textContent =
                "خطأ في الحفظ";
        }

        alert(
            "فشل حفظ إعدادات الصوت:\n" +
            error.message
        );

        return false;

    }
    finally {

        audioSaveInProgress = false;
    }
}


/* =========================================================
   EVENTS
   ========================================================= */

function initAudioEvents() {


    /* =====================================================
       QURAN
       ===================================================== */

    const quranType =
        getElement(
            "quranType"
        );

    if (quranType) {

        quranType.addEventListener(
            "change",
            function () {

                const selected =
                    getSelectedQuranFile();

                if (!selected) {
                    return;
                }

                quranSettings.selected =
                    selected.type;

                console.log(
                    "[QURAN] Selected:",
                    selected
                );
            }
        );
    }


    /* =====================================================
       VOLUMES
       ===================================================== */

    const volumeBindings = [

        {
            input: "volume",
            output: "volumeValue"
        },

        {
            input: "iqamaVolume",
            output: "iqamaVolumeValue"
        },

        {
            input: "morningAdhkarVolume",
            output: "morningAdhkarVolumeValue"
        },

        {
            input: "eveningAdhkarVolume",
            output: "eveningAdhkarVolumeValue"
        },

        {
            input: "kahfVolume",
            output: "kahfVolumeValue"
        },

        {
            input: "quranVolume",
            output: "quranVolumeValue"
        }
    ];


    volumeBindings.forEach(
        binding => {

            const input =
                getElement(
                    binding.input
                );

            if (!input) {
                return;
            }

            input.addEventListener(
                "input",
                function () {

                    showValue(
                        binding.output,
                        input.value
                    );
                }
            );
        }
    );
}


/* =========================================================
   GLOBAL FUNCTIONS
   ========================================================= */

window.audioPlay =
    audioPlay;

window.audioPause =
    audioPause;

window.audioStop =
    audioStop;

window.audioVolumeUp =
    audioVolumeUp;

window.audioVolumeDown =
    audioVolumeDown;

window.testAzan =
    testAzan;

window.testIqama =
    testIqama;

window.testMorningAdhkar =
    testMorningAdhkar;

window.testEveningAdhkar =
    testEveningAdhkar;

window.testKahf =
    testKahf;

window.testQuran =
    testQuran;

window.playFolder =
    playFolder;

window.saveVolume =
    saveVolume;

window.saveAudioSettings =
    saveAudioSettings;

window.loadAudioSettings =
    loadAudioSettings;

window.loadSelectedQuran =
    loadSelectedQuran;

window.readCurrentQuran =
    readCurrentQuran;

window.applyPreset =
    applyPreset;

window.applyIqamaPreset =
    applyIqamaPreset;


/* =========================================================
   STARTUP
   ========================================================= */

document.addEventListener(
    "DOMContentLoaded",
    async function () {

        console.log(
            "[AUDIO] Initializing..."
        );

        initAudioEvents();

        await loadAudioSettings();

        console.log(
            "[AUDIO] Ready"
        );
    }
);