#include "workspaceDaemon.h"

namespace KWin {

class KWIN_EXPORT MyEffectFactory : public PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginFactory_iid FILE "metadata.json")
    Q_INTERFACES(KWin::PluginFactory)

 public:
    explicit MyEffectFactory() = default;

    std::unique_ptr<Plugin> create() const override;
};

}  // namespace KWin

std::unique_ptr<KWin::Plugin> KWin::MyEffectFactory::create() const {
    return std::make_unique<ServiceRegister>();
}

#include "plugin.moc"
