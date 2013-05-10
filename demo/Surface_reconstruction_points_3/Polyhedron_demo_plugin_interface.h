#ifndef POLYHEDRON_DEMO_PLUGIN_INTERFACE_H
#define POLYHEDRON_DEMO_PLUGIN_INTERFACE_H

#include <QString>
#include <QList>
#include <QtPlugin>

class QAction;
class QMainWindow;
class Scene_interface;
class Messages_interface;

class Polyhedron_demo_plugin_interface 
{
public:
  virtual ~Polyhedron_demo_plugin_interface() {}
  virtual void init(QMainWindow*, Scene_interface*) {};
  virtual void init(QMainWindow* mw, Scene_interface* sc, Messages_interface*) {
    init(mw, sc);
  };
  virtual QList<QAction*> actions() const = 0;
};

Q_DECLARE_INTERFACE(Polyhedron_demo_plugin_interface,
                    "com.geometryfactory.PolyhedronDemo.PluginInterface/1.0")

#endif // POLYHEDRON_DEMO_PLUGIN_INTERFACE_H
