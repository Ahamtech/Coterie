/*
 * HubIntegration.hpp
 *
 *  Created on: 24 mai 2014
 *      Author: pierre
 */

#ifndef HUBINTEGRATION_HPP_
#define HUBINTEGRATION_HPP_

#include "HubAccount.hpp"


class HubIntegration : public HubAccount {
    Q_OBJECT;

public:
    HubIntegration(UDSUtil* udsUtil, HubCache* hubCache);
    virtual ~HubIntegration();    


    qint64 accountId();



    /*
     * Category ID of the Hub account.
     *
     * @returns qint64 last category Id
     */
    qint64 categoryId();

    void initializeCategories(QVariantList categories);

//public Q_SLOTS:

private:
    qint64 _categoryId;

};



#endif /* HUBINTEGRATION_HPP_ */
