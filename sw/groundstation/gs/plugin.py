import sys
import os
import logging

LOG = logging.getLogger('plugin')

class Plugin(object):
    def __init__(self, conf, source, message_file):
        pass

class PluginManager:
    def __init__(self, *plugin_dirs):
        #default plugin dir is ./plugins/
        if not plugin_dirs:
            plugin_dirs = [os.path.join(
                            os.path.dirname(os.path.realpath(__file__)),
                            "plugins")]

        for d in plugin_dirs:
            LOG.debug("Searching for plugins in %s" % d)
            plugin_files = [x[:-3] for x in os.listdir(d) if x.endswith(".py")]
            sys.path.insert(0, d)
            for plugin in plugin_files:
                LOG.debug("Importing %s" % plugin)
                try:
                    __import__(plugin)
                except Exception:
                    LOG.warn("Error loading plugin: %s" % plugin, exc_info=True)

        self._plugins = []

    def initialize_plugins(self, *args, **kwargs):
        for klass in Plugin.__subclasses__():
            try:
                self._plugins.append(klass(*args, **kwargs))
            except Exception:
                LOG.warn("Error instantiating plugin class: %s" % klass, exc_info=True)

    def get_plugins_implementing_interface(self, interface):
        return [p for p in self._plugins if isinstance(p, interface)]
