/*
 * Copyright (c) 2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UDSUTIL_HPP_
#define UDSUTIL_HPP_

#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <bb/pim/unified/unified_data_source.h>
#include <bb/pim/account/Account.hpp>
#include <bb/pim/account/Property>

class UDSUtil: public QObject {
    Q_OBJECT

public:
    /*
     * Constructor
     *
     * @param serviceURL          - unique service name
     * @param hubAssetsFolderName - asset folder name for Hub assets
     */
	UDSUtil(QString serviceURL, QString hubAssetsFolderName);

    /*
     *  Destructor.
     */
    virtual ~UDSUtil();

    /*
     *  This method registers as a UDS client and determines if a Hub reload is required.
     */
    void initialize();

    /*
     *  Initializes values for next IDs.
     */
    void initNextIds();

    /*
     *  Check whether the UDS initialization was performed and was successful.
     *
     *  @returns bool true if initialization was performed and successful, false if not
     */
    bool initialized();

    /*
     *  Check whether the UDS client registration was performed and was successful.
     *
     *  @returns bool true if registration was performed and successful, false if not
     */
    bool registered();

    /*
     *  Check whether the Hub account needs to be reloaded.
     *
     *  @returns bool true if Hub needs to be reloaded, false if not
     */
    bool reloadHub();

    /*
     *  Reset status for Hub reload.
     */
    void resetReloadHub();

    /*
     *   Restore values for next IDs from external cache.
     */
    Q_INVOKABLE bool restoreNextIds(qint64 nextAccountId, qint64 nextCategoryId, qint64 nextItemId);

    /*
     *   Add a new UDS account.
     *
     *   @param name        - account name
     *   @param displayName - display name
     *   @param serverName  - server name
     *   @param target      - invocation target for account items
     *   @param icon        - icon file name
     *   @param lockedIcon  - locked icon name
     *   @param composeIcon - compose icon name
     *   @param desc        - description
     *   @param compose     - support compose
     *   @param type        - account type
     *
     *  @returns qint64 accountId of new account or -1 if failed
     */
    Q_INVOKABLE qint64 addAccount(QString name, QString displayName, QString serverName, QString target,
    						QString icon, QString lockedIcon, QString composeIcon, QString desc,
    						bool compose, uds_account_type_t type);

    /*
     *   Update a UDS account.
     *
     *   @param accountId   - account ID
     *   @param name        - account name
     *   @param target      - invocation target for account items
     *   @param icon        - icon file name
     *   @param lockedIcon  - locked icon name
     *   @param composeIcon - compose icon name
     *   @param desc        - description
     *   @param compose     - support compose
     *   @param type        - account type
     *
     *  @returns bool true if operation successfal, false if not
     */
    Q_INVOKABLE bool updateAccount(qint64 accountId, QString name, QString target,
    								QString icon, QString lockedIcon, QString composeIcon, QString desc,
    								bool compose, uds_account_type_t type);

    /*
     *   Remove a UDS account.
     *
     *   @param accountId   - account ID
     *
     *  @returns bool true if operation successfal, false if not
     */
    Q_INVOKABLE bool removeAccount(qint64 accountId);

    /*
     *   Remove any pre-existing UDS / Accounts service accounts except for the one indicates.
     *   Note: This method is useful for cleaning up any accounts that might be left behind from a previous uninstall operation.
     *   or accidentally restored from a device restore operation.
     *
     *   @param accountId   - account ID
     *   @param name        - account name
     */
    Q_INVOKABLE void cleanupAccountsExcept(const qint64 accountId, const QString& name);

    /*
     *   Add a new category to an existing UDS account.
     *
     *   @param accountId        - account ID
     *   @param name             - category name
     *   @param parentCategoryId - parent categoryId ID
     *
     *  @returns qint64 categoryId of new category or -1 if failed
     */
    Q_INVOKABLE qint64 addCategory(qint64 accountId, QString name, qint64 parentCategoryId);

    /*
     *   Update an existing UDS account category.
     *
     *   @param accountId        - account ID
     *   @param categoryId       - category ID
     *   @param name             - category name
     *   @param parentCategoryId - parent categoryId ID
     *
     *  @returns bool true if operation successfal, false if not
     */
    Q_INVOKABLE bool updateCategory(qint64 accountId, qint64 categoryId, QString name, qint64 parentCategoryId);

    /*
     *   Remove a category from a UDS account.
     *
     *   @param accountId   - account ID
     *   @param categoryId   - category ID
     *
     *  @returns bool true if operation successfal, false if not
     */
    Q_INVOKABLE bool removeCategory(qint64 accountId, qint64 categoryId);

    /*
     *   Add an item to a UDS account.
     *
     *   @param accountId    - account ID
     *   @param categoryId   - category ID
     *   @param itemMap      - item map to update with various Hub item attributes
     *   @param name         - item name
     *   @param subject      - item subject
     *   @param mimeType     - item MIME type
     *   @param icon         - icon file name
     *   @param read         - true if the item is marked as read, false if otherwise
     *   @param syncId       - an ID to be used for syncing Hub items with external data
     *   @param userData     - extra data specific to the app that is associated with this item
     *   @param extendedData - extended data that controls special properties of Hub items
     *   @param timestamp    - the timestamp for the Hub item to be added
     *   @param contextState - context state
     *   @param notify       - true if a notification is to be generated, false otherwise
     *
     *  @returns qint64 item Id of new item or -1 if failed
     */
    Q_INVOKABLE qint64 addItem(qint64 accountId, qint64 categoryId, QVariantMap &itemMap, QString name, QString subject, QString mimeType, QString icon, bool read,
                                QString syncId, QString userData, QString extendedData,
                                long long timestamp, unsigned int contextState, bool notify);

    /*
     *   Update an item in a UDS account.
     *
     *   @param accountId    - account ID
     *   @param categoryId   - category ID
     *   @param itemMap      - item map to update with various Hub item attributes
     *   @param srcId        - source ID for item to be updated
     *   @param name         - item name
     *   @param subject      - item subject
     *   @param mimeType     - item MIME type
     *   @param icon         - icon file name
     *   @param read         - true if the item is marked as read, false if otherwise
     *   @param syncId       - an ID to be used for syncing Hub items with external data
     *   @param userData     - extra data specific to the app that is associated with this item
     *   @param extendedData - extended data that controls special properties of Hub items
     *   @param timestamp    - the timestamp for the Hub item to be added
     *   @param contextState - context state
     *   @param notify       - true if a notification is to be generated, false otherwise
     *
     *  @returns bool true if operation successfal, false if not
     */
    Q_INVOKABLE bool updateItem(qint64 accountId, qint64 categoryId, QVariantMap &itemMap, QString srcId, QString name, QString subject, QString mimeType, QString icon, bool read,
            QString syncId, QString userData, QString extendedData,
            long long timestamp, unsigned int contextState, bool notify);

    /*
     *   Removes an item from a UDS account.
     *
     *   @param accountId    - account ID
     *   @param categoryId   - category ID
     *   @param srcId        - source ID for item to be removed
     *
     *  @returns bool true if operation successfal, false if not
     */
    Q_INVOKABLE bool removeItem(qint64 accountId, qint64 categoryId, QString srcId);

    /*
     *   Add a UDS account action.
     *
     *   @param accountId     - account ID
     *   @param action        - invocation action
     *   @param title         - action title
     *   @param invtarget     - invocation target
     *   @param invtargettype - invocation target target
     *   @param imgsource     - action image file name
     *   @param mimeType      - invocation MIME type
     *   @param placement     - placement of action ie. on action bar or context menu
     */
    Q_INVOKABLE bool addAccountAction(qint64 accountId, QString action, QString title,
								     QString invtarget, QString invtargettype, QString imgsource,
									 QString mimetype,int placement);

    /*
     *   Update a UDS account action.
     *
     *   @param accountId     - account ID
     *   @param action        - invocation action
     *   @param title         - action title
     *   @param invtarget     - invocation target
     *   @param invtargettype - invocation target target
     *   @param imgsource     - action image file name
     *   @param mimeType      - invocation MIME type
     *   @param placement     - placement of action ie. on action bar or context menu
     */
    Q_INVOKABLE bool updateAccountAction(qint64 accountId, QString action, QString title,
										QString invtarget, QString invtargettype, QString imgsource,
										QString mimetype,int placement);

    /*
     *   Add a UDS item action.
     *
     *   @param accountId     - account ID
     *   @param action        - invocation action
     *   @param title         - action title
     *   @param invtarget     - invocation target
     *   @param invtargettype - invocation target target
     *   @param imgsource     - action image file name
     *   @param mimeType      - invocation MIME type
     *   @param placement     - placement of action ie. on action bar or context menu
     */
    Q_INVOKABLE bool addItemAction(qint64 accountId, QString action, QString title,
								 QString invtarget, QString invtargettype, QString imgsource,
								 QString mimetype,int placement);

    /*
     *   Update a UDS item action.
     *
     *   @param accountId     - account ID
     *   @param action        - invocation action
     *   @param title         - action title
     *   @param invtarget     - invocation target
     *   @param invtargettype - invocation target target
     *   @param imgsource     - action image file name
     *   @param mimeType      - invocation MIME type
     *   @param placement     - placement of action ie. on action bar or context menu
     */
    Q_INVOKABLE bool updateItemAction(qint64 accountId, QString action, QString title,
									 QString invtarget, QString invtargettype, QString imgsource,
									 QString mimetype,int placement);

private:
    uds_perimeter_type_t _itemPerimeterType;
    uds_context_t _udsHandle;

    int _nextAccountId;
    int _nextCategoryId;
    int _nextItemId;

    bool _async;
    bool _isInitializationSuccess;
    bool _isRegistrationSuccess;
    bool _reloadHub;

    char _assetPath[256];
    char _serviceURL[256];
};

#endif /* UDSUTIL_HPP_ */
