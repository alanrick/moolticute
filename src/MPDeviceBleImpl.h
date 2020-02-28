#ifndef MPDEVICEBLEIMPL_H
#define MPDEVICEBLEIMPL_H

#include "MPDevice.h"
#include "BleCommon.h"
#include "MPBLEFreeAddressProvider.h"

class MessageProtocolBLE;

/**
 * @brief The MPDeviceBleImpl class
 * Implementations of only BLE related commands
 * and helper functions
 */
class MPDeviceBleImpl: public QObject
{
    Q_OBJECT

    QT_WRITABLE_PROPERTY(QString, mainMCUVersion, QString())
    QT_WRITABLE_PROPERTY(QString, auxMCUVersion, QString())
    QT_WRITABLE_PROPERTY(bool, loginPrompt, false)
    QT_WRITABLE_PROPERTY(bool, PINforMMM, false)
    QT_WRITABLE_PROPERTY(bool, storagePrompt, false)
    QT_WRITABLE_PROPERTY(bool, advancedMenu, false)
    QT_WRITABLE_PROPERTY(bool, bluetoothEnabled, false)
    QT_WRITABLE_PROPERTY(bool, knockDisabled, false)

    enum UserSettingsMask : quint8
    {
        LOGIN_PROMPT      = 0x01,
        PIN_FOR_MMM       = 0x02,
        STORAGE_PROMPT    = 0x04,
        ADVANCED_MENU     = 0x08,
        BLUETOOTH_ENABLED = 0x10,
        KNOCK_DISABLED    = 0x20
    };

    struct FreeAddressInfo
    {
        QByteArray addressPackage = {};
        int parentNodeRequested = 0;
        int childNodeRequested = 0;
        int startingPosition = 0;
    };

public:
    MPDeviceBleImpl(MessageProtocolBLE *mesProt, MPDevice *dev);

    bool isFirstPacket(const QByteArray &data);
    bool isLastPacket(const QByteArray &data);

    void getPlatInfo();
    void getDebugPlatInfo(const MessageHandlerCbData &cb);
    QVector<int> calcDebugPlatInfo(const QByteArray &platInfo);

    void flashMCU(const MessageHandlerCb &cb);
    void uploadBundle(QString filePath, const MessageHandlerCb &cb, const MPDeviceProgressCb &cbProgress);
    void fetchData(QString filePath, MPCmd::Command cmd);
    inline void stopFetchData() { fetchState = Common::FetchState::STOPPED; }

    void storeCredential(const BleCredential &cred, MessageHandlerCb cb);
    void getCredential(const QString& service, const QString& login, const QString& reqid, const QString& fallbackService, const MessageHandlerCbData &cb);
    void getFallbackServiceCredential(AsyncJobs *jobs, const QString& fallbackService, const QString& login, const MessageHandlerCbData &cb);
    BleCredential retrieveCredentialFromResponse(QByteArray response, QString service, QString login) const;

    void sendResetFlipBit();
    void setCurrentFlipBit(QByteArray &msg);
    void flipMessageBit(QVector<QByteArray> &msg);
    void flipMessageBit(QByteArray &msg);
    /**
     * @brief processReceivedData
     * @param data actual packet from the device
     * @param dataReceived full message data
     * @return true if data is last packet of the message
     */
    bool processReceivedData(const QByteArray& data, QByteArray& dataReceived);

    QVector<QByteArray> processReceivedStartNodes(const QByteArray& data) const;

    bool isAfterAuxFlash();

    void getUserCategories(const MessageHandlerCbData &cb);
    void setUserCategories(const QJsonObject &categories, const MessageHandlerCbData &cb);
    void fillGetCategory(const QByteArray& data, QJsonObject &categories);
    void fetchCategories();
    QByteArray createUserCategoriesMsg(const QJsonObject &categories);

    void readUserSettings(const QByteArray& settings);
    void sendUserSettings();

    void processDebugMsg(const QByteArray& data, bool& isDebugMsg);
    MPBLEFreeAddressProvider& getFreeAddressProvider() { return freeAddressProv; }

    void updateChangeNumbers(AsyncJobs *jobs, quint8 flags);

    QJsonObject getUserCategories() const { return m_categories; }
    void updateUserCategories(const QJsonObject &categories);
    bool isUserCategoriesChanged(const QJsonObject &categories) const;
    void setImportUserCategories(const QJsonObject &categories);
    void importUserCategories();
    void setNodeCategory(MPNode* node, int category);
    void setNodeKeyAfterLogin(MPNode* node, int key);
    void setNodeKeyAfterPwd(MPNode* node, int key);

    QList<QByteArray> getFavorites(const QByteArray& data);

    QByteArray getStartAddressToSet(const QVector<QByteArray>& startNodeArray, Common::AddressType addrType) const;

    void readLanguages();

    void loadWebAuthnNodes(AsyncJobs * jobs, const MPDeviceProgressCb &cbProgress);
    void appendLoginNode(MPNode* loginNode, MPNode* loginNodeClone, Common::AddressType addrType);
    void appendLoginChildNode(MPNode* loginChildNode, MPNode* loginChildNodeClone, Common::AddressType addrType);
    void generateExportData(QJsonArray& exportTopArray);

signals:
    void userSettingsChanged(QJsonObject settings);
    void bleDeviceLanguage(const QJsonObject& langs);
    void bleKeyboardLayout(const QJsonObject& layouts);

private slots:
    void handleLongMessageTimeout();

private:
    void checkDataFlash(const QByteArray &data, QElapsedTimer *timer, AsyncJobs *jobs, QString filePath, const MPDeviceProgressCb &cbProgress);
    void sendBundleToDevice(QString filePath, AsyncJobs *jobs, const MPDeviceProgressCb &cbProgress);
    void writeFetchData(QFile *file, MPCmd::Command cmd);
    inline bool isBundleFileReadable(const QString& filePath);

    QByteArray createStoreCredMessage(const BleCredential &cred);
    QByteArray createGetCredMessage(QString service, QString login);
    QByteArray createCheckCredMessage(const BleCredential &cred);
    QByteArray createCredentialMessage(const CredMap &credMap);

    inline void flipBit();
    void resetFlipBit();

    MessageProtocolBLE *bleProt;
    MPDevice *mpDev;
    Common::FetchState fetchState = Common::FetchState::STOPPED;

    quint8 m_flipBit = 0x00;
    quint8 m_currentUserSettings = 0x00;
    QString m_debugMsg = "";

    MPBLEFreeAddressProvider freeAddressProv;
    QJsonObject m_categories;
    QJsonObject m_categoriesToImport;
    bool m_categoriesFetched = false;
    QJsonObject m_deviceLanguages;
    QJsonObject m_keyboardLayouts;

    static constexpr quint8 MESSAGE_FLIP_BIT = 0x80;
    static constexpr quint8 MESSAGE_ACK_AND_PAYLOAD_LENGTH = 0x7F;
    static constexpr int BUNBLE_DATA_WRITE_SIZE = 256;
    static constexpr int BUNBLE_DATA_ADDRESS_SIZE = 4;
    static constexpr int USER_CATEGORY_COUNT = 4;
    static constexpr int USER_CATEGORY_LENGTH = 66;
    static constexpr int FAV_DATA_SIZE = 4;
    static constexpr int FAV_NUMBER = 50;
    static constexpr int LONG_MESSAGE_TIMEOUT_MS = 2000;
    static constexpr int FIRST_PACKET_PAYLOAD_SIZE = 58;
    static constexpr int INVALID_LAYOUT_LANG_SIZE = 0xFFFF;
    const QString AFTER_AUX_FLASH_SETTING = "settings/after_aux_flash";
    const static char ZERO_BYTE = static_cast<char>(0x00);
};

#endif // MPDEVICEBLEIMPL_H
