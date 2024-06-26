/*
 * Fooyin
 * Copyright © 2023, Luke Taylor <LukeT1@proton.me>
 *
 * Fooyin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fooyin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fooyin.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <utils/actions/actionmanager.h>

#include "actions/actioncommand.h"
#include "menucontainer.h"

#include <utils/actions/command.h>
#include <utils/settings/settingsmanager.h>

#include <QAction>
#include <QApplication>
#include <QIODevice>
#include <QMainWindow>
#include <QMenuBar>

#include <set>

namespace Fooyin {
struct ActionManager::Private
{
    ActionManager* self;

    SettingsManager* settingsManager;
    QMainWindow* mainWindow{nullptr};

    Context currentContext;
    bool contextOverride{false};
    WidgetContext* widgetOverride{nullptr};

    std::unordered_map<Id, std::unique_ptr<ActionCommand>, Id::IdHash> idCmdMap;
    std::unordered_map<Id, std::unique_ptr<MenuContainer>, Id::IdHash> idContainerMap;
    std::set<MenuContainer*> scheduledContainerUpdates;

    WidgetContextList activeContext;
    std::unordered_map<QWidget*, WidgetContext*> contextWidgets;

    explicit Private(ActionManager* self_, SettingsManager* settingsManager_)
        : self{self_}
        , settingsManager{settingsManager_}
    { }

    ActionCommand* overridableAction(const Id& id)
    {
        if(idCmdMap.contains(id)) {
            return idCmdMap.at(id).get();
        }

        auto* command = idCmdMap.emplace(id, std::make_unique<ActionCommand>(id)).first->second.get();
        loadSetting(id, command);
        QAction* action = command->action();
        mainWindow->addAction(action);
        action->setObjectName(id.name());
        action->setShortcutContext(Qt::ApplicationShortcut);
        command->setCurrentContext(currentContext);

        return command;
    }

    void loadSetting(const Id& id, Command* command) const
    {
        const QString key = QStringLiteral("KeyboardShortcuts/") + id.name();

        if(settingsManager->fileContains(key)) {
            const QVariant var = settingsManager->fileValue(key);
            if(QMetaType::Type(var.typeId()) == QMetaType::QStringList) {
                ShortcutList shortcuts;
                std::ranges::transform(var.toStringList(), std::back_inserter(shortcuts),
                                       [](const QKeySequence& k) { return k.toString(); });
                command->setShortcut(shortcuts);
            }
            else {
                command->setShortcut({QKeySequence::fromString(var.toString())});
            }
        }
    }

    void updateContainer()
    {
        for(MenuContainer* container : scheduledContainerUpdates) {
            container->update();
        }
        scheduledContainerUpdates.clear();
    }

    void scheduleContainerUpdate(MenuContainer* actionContainer)
    {
        const bool needsSchedule = scheduledContainerUpdates.empty();
        scheduledContainerUpdates.emplace(actionContainer);
        if(needsSchedule) {
            QMetaObject::invokeMethod(
                self, [this]() { updateContainer(); }, Qt::QueuedConnection);
        }
    }

    void updateContextObject(const WidgetContextList& context)
    {
        activeContext = context;

        Context uniqueContexts;
        for(WidgetContext* ctx : activeContext) {
            for(const Id& id : ctx->context()) {
                uniqueContexts.append(id);
            }
        }

        uniqueContexts.append(Constants::Context::Global);

        setContext(uniqueContexts);
        emit self->contextChanged(uniqueContexts);
    }

    void updateFocusWidget(QWidget* widget)
    {
        if(qobject_cast<QMenuBar*>(widget) || qobject_cast<QMenu*>(widget)) {
            return;
        }

        if(contextOverride) {
            return;
        }

        WidgetContextList newContext;

        if(QWidget* focusedWidget = QApplication::focusWidget()) {
            while(focusedWidget) {
                if(auto* widgetContext = self->contextObject(focusedWidget)) {
                    if(widgetContext->isEnabled()) {
                        newContext.push_back(widgetContext);
                    }
                }
                focusedWidget = focusedWidget->parentWidget();
            }
        }

        if(!newContext.empty() || QApplication::focusWidget() == mainWindow->focusWidget()) {
            updateContextObject(newContext);
        }
    }

    void setContext(const Context& updatedContext)
    {
        currentContext = updatedContext;
        for(const auto& [id, command] : idCmdMap) {
            command->setCurrentContext(currentContext);
        }
    }
};

ActionManager::ActionManager(SettingsManager* settingsManager, QObject* parent)
    : QObject{parent}
    , p{std::make_unique<Private>(this, settingsManager)}
{
    QObject::connect(qApp, &QApplication::focusChanged, this,
                     [this](QWidget* /*old*/, QWidget* now) { p->updateFocusWidget(now); });
}

ActionManager::~ActionManager()
{
    QObject::disconnect(qApp, &QApplication::focusChanged, this, nullptr);

    for(auto [_, context] : p->contextWidgets) {
        context->disconnect();
    }

    p->contextWidgets.clear();
    p->activeContext.clear();

    for(const auto& [_, container] : p->idContainerMap) {
        container->disconnect();
    }

    p->idContainerMap.clear();
    p->idCmdMap.clear();
}

void ActionManager::setMainWindow(QMainWindow* mainWindow)
{
    p->mainWindow = mainWindow;
}

void ActionManager::saveSettings()
{
    for(const auto& [_, command] : p->idCmdMap) {
        const QString key = QStringLiteral("KeyboardShortcuts/") + command->id().name();

        const ShortcutList commandShortcuts = command->shortcuts();
        const ShortcutList defaultShortcuts = command->defaultShortcuts();
        if(commandShortcuts != defaultShortcuts) {
            // Only save user changes
            QStringList keys;
            std::ranges::transform(commandShortcuts, std::back_inserter(keys),
                                   [](const QKeySequence& k) { return k.toString(); });
            p->settingsManager->fileSet(key, keys);
        }
        else {
            p->settingsManager->fileRemove(key);
        }
    }
}

WidgetContext* ActionManager::currentContextObject() const
{
    return p->activeContext.empty() ? nullptr : p->activeContext.front();
}

QWidget* ActionManager::currentContextWidget() const
{
    WidgetContext* context = currentContextObject();
    return context ? context->widget() : nullptr;
}

WidgetContext* ActionManager::contextObject(QWidget* widget) const
{
    if(p->contextWidgets.contains(widget)) {
        return p->contextWidgets.at(widget);
    }
    return nullptr;
}

void ActionManager::addContextObject(WidgetContext* context)
{
    if(!context) {
        return;
    }

    QWidget* widget = context->widget();
    if(p->contextWidgets.contains(widget)) {
        return;
    }

    p->contextWidgets.emplace(widget, context);
    QObject::connect(context, &WidgetContext::isEnabledChanged, this,
                     [this] { p->updateFocusWidget(QApplication::focusWidget()); });
    QObject::connect(context, &QObject::destroyed, this, [this, context] { removeContextObject(context); });
}

void ActionManager::overrideContext(WidgetContext* context, bool override)
{
    if(!context) {
        return;
    }

    if(override && p->contextOverride) {
        // Only one override allowed
        return;
    }

    if(override) {
        p->contextOverride = true;
        p->widgetOverride  = context;
        p->updateContextObject({context});
    }
    else if(context == p->widgetOverride) {
        p->contextOverride = false;
        p->widgetOverride  = nullptr;
        p->activeContext.clear();
        p->currentContext = {};
        p->updateFocusWidget(QApplication::focusWidget());
    }
}

void ActionManager::removeContextObject(WidgetContext* context)
{
    if(!context) {
        return;
    }

    QObject::disconnect(context, &QObject::destroyed, this, nullptr);

    if(!std::erase_if(p->contextWidgets, [context](const auto& v) { return v.second == context; })) {
        return;
    }

    if(std::erase_if(p->activeContext, [context](WidgetContext* wc) { return wc == context; }) > 0) {
        p->updateContextObject(p->activeContext);
    }
}

ActionContainer* ActionManager::createMenu(const Id& id)
{
    if(p->idContainerMap.contains(id)) {
        return p->idContainerMap.at(id).get();
    }

    auto* menu = p->idContainerMap.emplace(id, std::make_unique<MenuActionContainer>(id, this)).first->second.get();

    QObject::connect(menu, &MenuContainer::requestUpdate, this,
                     [this](MenuContainer* container) { p->scheduleContainerUpdate(container); });

    menu->appendGroup(Actions::Groups::One);
    menu->appendGroup(Actions::Groups::Two);
    menu->appendGroup(Actions::Groups::Three);

    return menu;
}

ActionContainer* ActionManager::createMenuBar(const Id& id)
{
    if(p->idContainerMap.contains(id)) {
        return p->idContainerMap.at(id).get();
    }

    {
        auto* menuBar = new QMenuBar(p->mainWindow);
        menuBar->setObjectName(id.name());

        auto container = std::make_unique<MenuBarActionContainer>(id, this);
        container->setMenuBar(menuBar);
        p->idContainerMap.emplace(id, std::move(container));
    }

    auto* menuBar = p->idContainerMap.at(id).get();

    QObject::connect(menuBar, &MenuContainer::requestUpdate, this,
                     [this](MenuContainer* container) { p->scheduleContainerUpdate(container); });

    return menuBar;
}

Command* ActionManager::registerAction(QAction* action, const Id& id, const Context& context)
{
    ActionCommand* command = p->overridableAction(id);
    if(command) {
        command->addOverrideAction(action, context, !p->contextOverride);
        emit commandsChanged();
    }
    return command;
}

Command* ActionManager::command(const Id& id) const
{
    if(p->idCmdMap.contains(id)) {
        return p->idCmdMap.at(id).get();
    }
    return nullptr;
}

CommandList ActionManager::commands() const
{
    CommandList commands;
    std::ranges::transform(std::as_const(p->idCmdMap), std::back_inserter(commands),
                           [&](const auto& idCommand) { return idCommand.second.get(); });
    return commands;
}

ActionContainer* ActionManager::actionContainer(const Id& id) const
{
    if(p->idContainerMap.contains(id)) {
        return p->idContainerMap.at(id).get();
    }
    return nullptr;
}
} // namespace Fooyin

#include "utils/actions/moc_actionmanager.cpp"
