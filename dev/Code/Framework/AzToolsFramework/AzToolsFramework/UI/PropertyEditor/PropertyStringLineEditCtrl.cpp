/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include "StdAfx.h"
#include "PropertyStringLineEditCtrl.hxx"
#include "PropertyQTConstants.h"
#include <AzQtComponents/Components/StyledLineEdit.h>
#include <QtWidgets/QHBoxLayout>
#include <QFocusEvent>

namespace AzToolsFramework
{
    PropertyStringLineEditCtrl::PropertyStringLineEditCtrl(QWidget* pParent)
        : QWidget(pParent)
    {
        // create the gui, it consists of a layout, and in that layout, a text field for the value
        // and then a slider for the value.
        QHBoxLayout* pLayout = new QHBoxLayout(this);
        m_pLineEdit = new AzQtComponents::StyledLineEdit(this);

        pLayout->setSpacing(4);
        pLayout->setContentsMargins(1, 0, 1, 0);

        pLayout->addWidget(m_pLineEdit);

        m_pLineEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        m_pLineEdit->setMinimumWidth(PropertyQTConstant_MinimumWidth);
        m_pLineEdit->setFixedHeight(PropertyQTConstant_DefaultHeight);

        m_pLineEdit->setFocusPolicy(Qt::StrongFocus);

        setLayout(pLayout);
        setFocusProxy(m_pLineEdit);
        setFocusPolicy(m_pLineEdit->focusPolicy());

        ConnectWidgets();
    };

    void PropertyStringLineEditCtrl::setValue(AZStd::string& value)
    {
        QString text = m_pLineEdit->text();
        if (text.compare(value.data()) != 0)
        {
            m_pLineEdit->blockSignals(true);
            m_pLineEdit->setText(value.c_str());
            m_pLineEdit->blockSignals(false);
        }
    }

    void PropertyStringLineEditCtrl::focusInEvent(QFocusEvent* e)
    {
        m_pLineEdit->event(e);
        m_pLineEdit->selectAll();
    }

    AZStd::string PropertyStringLineEditCtrl::value() const
    {
        return AZStd::string(m_pLineEdit->text().toUtf8().data());
    }

    void PropertyStringLineEditCtrl::setMaxLen(int maxLen)
    {
        m_pLineEdit->blockSignals(true);
        m_pLineEdit->setMaxLength(maxLen);
        m_pLineEdit->blockSignals(false);
    }

    void PropertyStringLineEditCtrl::onChildLineEditValueChange(const QString& newValue)
    {
        AZStd::string changedVal(newValue.toUtf8().data());
        emit valueChanged(changedVal);
    }

    PropertyStringLineEditCtrl::~PropertyStringLineEditCtrl()
    {
    }


    QWidget* PropertyStringLineEditCtrl::GetFirstInTabOrder()
    {
        return m_pLineEdit;
    }
    QWidget* PropertyStringLineEditCtrl::GetLastInTabOrder()
    {
        return m_pLineEdit;
    }

    void PropertyStringLineEditCtrl::UpdateTabOrder()
    {
        // There's only one QT widget on this property.
    }

    void PropertyStringLineEditCtrl::ConnectWidgets()
    {
        connect(m_pLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onChildLineEditValueChange(const QString&)));
        connect(m_pLineEdit, &QLineEdit::editingFinished, this, [this]()
        {
            PropertyEditorGUIMessages::Bus::Broadcast(&PropertyEditorGUIMessages::Bus::Handler::OnEditingFinished, this);
        });
    }

    QWidget* StringPropertyLineEditHandler::CreateGUI(QWidget* pParent)
    {
        PropertyStringLineEditCtrl* newCtrl = aznew PropertyStringLineEditCtrl(pParent);
        connect(newCtrl, &PropertyStringLineEditCtrl::valueChanged, this, [newCtrl]()
            {
                EBUS_EVENT(PropertyEditorGUIMessages::Bus, RequestWrite, newCtrl);
            });
        return newCtrl;
    }

    void StringPropertyLineEditHandler::ConsumeAttribute(PropertyStringLineEditCtrl* GUI, AZ::u32 attrib, PropertyAttributeReader* attrValue, const char* debugName)
    {
         Q_UNUSED(debugName);

        GUI->blockSignals(true);
        if (attrib == AZ::Edit::Attributes::MaxLength)
        {
            AZ::s64 value;
            if (attrValue->Read<int>(value))
            {
                GUI->setMaxLen(value);
            }
            else
            {
                AZ_WarningOnce("AzToolsFramework", false, "Failed to read 'MaxLength' attribute from property '%s' into text field", debugName);
            }
        }
        GUI->blockSignals(false);
    }

    void StringPropertyLineEditHandler::WriteGUIValuesIntoProperty(size_t index, PropertyStringLineEditCtrl* GUI, property_t& instance, InstanceDataNode* node)
    {
        (int)index;
        (void)node;
        AZStd::string val = GUI->value();
        instance = static_cast<property_t>(val);
    }

    bool StringPropertyLineEditHandler::ReadValuesIntoGUI(size_t index, PropertyStringLineEditCtrl* GUI, const property_t& instance, InstanceDataNode* node)
    {
        (int)index;
        (void)node;
        AZStd::string val = instance;
        GUI->setValue(val);
        return false;
    }

    void RegisterStringLineEditHandler()
    {
        EBUS_EVENT(PropertyTypeRegistrationMessages::Bus, RegisterPropertyType, aznew StringPropertyLineEditHandler());
    }
}

#include <UI/PropertyEditor/PropertyStringLineEditCtrl.moc>