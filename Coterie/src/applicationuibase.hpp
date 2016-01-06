/*
 * Copyright (c) 2013-2014 BlackBerry Limited.
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

#ifndef APPLICATIONUIBASE_H_
#define APPLICATIONUIBASE_H_

#include <QObject>

namespace bb {
    namespace system {
      class InvokeManager;
    }
    namespace cascades {
      class LocaleHandler;
    }
}
class QTranslator;

class ApplicationUIBase : public QObject
{
    Q_OBJECT
public:
    ApplicationUIBase(bb::system::InvokeManager* invokeManager);
    virtual ~ApplicationUIBase();
private slots:
    void onSystemLanguageChanged();
protected:
    bb::system::InvokeManager* m_pInvokeManager;
private:
    QTranslator* m_translator;
    bb::cascades::LocaleHandler* m_pLocaleHandler;
};

#endif /* APPLICATIONUIBASE_H_ */
