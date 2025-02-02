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

#pragma once

#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>
#include <EMotionFX/Source/AnimGraphBus.h>
#include <EMotionStudio/Plugins/StandardPlugins/Source/StandardPluginsConfig.h>
#include <QPointer>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QScrollArea)
QT_FORWARD_DECLARE_CLASS(QTreeWidget)
QT_FORWARD_DECLARE_CLASS(QTreeWidgetItem)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)


namespace EMotionFX
{
    class GroupParameter;
    class ValueParameter;
}

namespace AzQtComponents
{
    class FilteredSearchWidget;
}

namespace EMStudio
{
    class AnimGraphPlugin;
    class ValueParameterEditor;
    
    class ParameterCreateRenameWindow
        : public QDialog
    {
        Q_OBJECT
        MCORE_MEMORYOBJECTCATEGORY(ParameterCreateRenameWindow, EMFX_DEFAULT_ALIGNMENT, MEMCATEGORY_STANDARDPLUGINS_ANIMGRAPH)

    public:
        ParameterCreateRenameWindow(const char* windowTitle, const char* topText, const char* defaultName, const char* oldName, const AZStd::vector<AZStd::string>& invalidNames, QWidget* parent);
        virtual ~ParameterCreateRenameWindow();

        AZStd::string GetName() const
        {
            return mLineEdit->text().toUtf8().data();
        }

    private slots:
        void NameEditChanged(const QString& text);

    private:
        AZStd::string                   mOldName;
        AZStd::vector<AZStd::string>    mInvalidNames;
        QPushButton*                    mOKButton;
        QPushButton*                    mCancelButton;
        QLineEdit*                      mLineEdit;
    };

    class ParameterWindow
        : public QWidget
        , private AzToolsFramework::IPropertyEditorNotify
        , protected EMotionFX::AnimGraphNotificationBus::Handler
    {
        Q_OBJECT
        MCORE_MEMORYOBJECTCATEGORY(ParameterWindow, EMFX_DEFAULT_ALIGNMENT, MEMCATEGORY_STANDARDPLUGINS_ANIMGRAPH);

    public:
        ParameterWindow(AnimGraphPlugin* plugin);
        ~ParameterWindow();

        void Reinit(bool forceReinit = false);

        // retrieve current selection
        const EMotionFX::Parameter* GetSingleSelectedParameter() const;

        bool GetIsParameterSelected(const AZStd::string& parameterName)
        {
            if (AZStd::find(mSelectedParameterNames.begin(), mSelectedParameterNames.end(), parameterName) == mSelectedParameterNames.end())
            {
                return false;
            }
            return true;
        }

        // select manually
        void SingleSelectGroupParameter(const char* groupName, bool ensureVisibility = false, bool updateInterface = false);

        void UpdateParameterValue(const EMotionFX::Parameter* parameter);

        void UpdateParameterValues();

        void ClearParameters(bool showConfirmationDialog = true);

        // AnimGraphNotificationBus
        void OnParameterActionTriggered(const EMotionFX::ValueParameter* valueParameter) override;

    public slots:
        void OnRecorderStateChanged();

    private slots:
        void UpdateInterface();

        void OnAddParameter();
        void OnEditButton();
        void OnRemoveButton();
        void OnClearButton() { ClearParameters(); }

        void OnGroupCollapsed(QTreeWidgetItem* item);
        void OnGroupExpanded(QTreeWidgetItem* item);

        void OnAddGroup();
        void OnGroupParameterSelected();

        void OnMoveParameterUp();
        void OnMoveParameterDown();
        void OnTextFilterChanged(const QString& text);
        void OnSelectionChanged();
        void OnMakeDefaultValue();

        void OnGamepadControlToggle();

    private:
        void contextMenuEvent(QContextMenuEvent* event) override;
        void CanMove(bool* outMoveUpPossible, bool* outMoveDownPossible);

        void UpdateSelectionArrays();

        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

        void AddParameterToInterface(EMotionFX::AnimGraph* animGraph, const EMotionFX::Parameter* parameter, QTreeWidgetItem* parentWidgetItem);

        /**
         * Update the attributes the parameter widgets modify when they are being changed in the interface.
         * Iterate through the selected actor instances and check for the ones that are running the anim graph whose parameters
         * we're showing. This will make sure that e.g. when changing a slider in the parameter window, the corresponding
         * anim graph instances get informed about the changed values.
         */
        void UpdateAttributesForParameterWidgets();

        /**
         * Get the attributes for the given parameter that are influenced by any of the currently selected actor instances
         * that are running the anim graph whose parameters we're showing.
         * @param[in] parameterIndex The parameter index to get the influenced attributes for.
         * @result The list of influenced attributes from the anim graph instances.
         */
        AZStd::vector<MCore::Attribute*> GetAttributesForParameter(size_t parameterIndex) const;

        /**
         * Check if the gamepad control mode is enabled for the given parameter and if its actually being controlled or not.
         */
        void GetGamepadState(EMotionFX::AnimGraph* animGraph, const EMotionFX::Parameter* parameter, bool* outIsActuallyControlled, bool* outIsEnabled);
        void SetGamepadState(EMotionFX::AnimGraph* animGraph, const EMotionFX::Parameter* parameter, bool isEnabled);
        void SetGamepadButtonTooltip(QPushButton* button);

        // IPropertyEditorNotify
        void BeforePropertyModified(AzToolsFramework::InstanceDataNode* pNode) override;
        void AfterPropertyModified(AzToolsFramework::InstanceDataNode* pNode) override;
        void SetPropertyEditingActive(AzToolsFramework::InstanceDataNode* pNode) override;
        void SetPropertyEditingComplete(AzToolsFramework::InstanceDataNode* pNode) override;
        void SealUndoStack() override;
        void RequestPropertyContextMenu(AzToolsFramework::InstanceDataNode*, const QPoint&) override;
        void PropertySelectionChanged(AzToolsFramework::InstanceDataNode *, bool) override;

        EMotionFX::AnimGraph*           m_animGraph;

        // toolbar buttons
        QPushButton*                    mAddButton;
        static int                      m_contextMenuWidth;
        QPushButton*                    mRemoveButton;
        QPushButton*                    mClearButton;

        QPushButton*                    mEditButton;
        QPushButton*                    mMoveUpButton;
        QPushButton*                    mMoveDownButton;

        AZStd::vector<AZStd::string>    mSelectedParameterNames;
        bool                            mEnsureVisibility;
        bool                            mLockSelection;

        AZStd::string                   mFilterString;
        AnimGraphPlugin*                mPlugin;
        QTreeWidget*                    mTreeWidget;
        AzQtComponents::FilteredSearchWidget* m_searchWidget;
        QVBoxLayout*                    mVerticalLayout;
        QScrollArea*                    mScrollArea;
        AZStd::string                   mNameString;
        struct ParameterWidget
        {
            AZStd::unique_ptr<ValueParameterEditor> m_valueParameterEditor;
            QPointer<AzToolsFramework::ReflectedPropertyEditor> m_propertyEditor;
        };
        typedef AZStd::unordered_map<const EMotionFX::Parameter*, ParameterWidget> ParameterWidgetByParameter;
        ParameterWidgetByParameter m_parameterWidgets;
    };
} // namespace EMStudio
