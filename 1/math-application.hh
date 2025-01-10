#ifndef MATH_APPLICATION_HH_INCLUDED
#define MATH_APPLICATION_HH_INCLUDED


#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/paned.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/cellrendererprogress.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm.h>
#include <gtkmm/stock.h>
#include <gtkmm/actiongroup.h>

#include "math.hh"
#include "worker.hh"

namespace oct::ec::v1
{
    class Application : public Gtk::Window
    {
    public:
        enum class Layout
        {
            none,
            left_paned,

        };

    public:
        Application();
        virtual ~Application();

        void init();

    protected:
        //Signal handlers:
        void on_button_clicked();

    private:
        Layout layout;

    protected:
        //Member widgets:
        Gtk::Box box_main;
        Gtk::Box box_top_panel;
        Gtk::Box box_work;
        Gtk::Button m_button;
        Gtk::ScrolledWindow paned_scroll;
        Gtk::Paned paned;



    };

    class Panel : public Gtk::TreeView
    {
    };

    class GroupPanel : public Panel
    {
    public:
        class ModelColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:

            ModelColumns()
            {
                add(index);
                add(evaluation);
            }

            Gtk::TreeModelColumn<size_t> index;
            Gtk::TreeModelColumn<double> evaluation;
        };

        void load(const BinoprGroup<3,double,Binopr>& );
        void clear();
    public:
        GroupPanel();

    protected:
            ModelColumns columns;
            Glib::RefPtr<Gtk::ListStore> ref_tree;
    };


    class EC : public Application
    {
    public:
        EC();
        virtual ~EC();

    private:

    };

    class MathEC : public EC
    {
    public:
        MathEC();
        virtual ~MathEC();



    private:
        GroupPanel group_tree;
        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
        Gtk::Menu* m_pMenuPopup;
        //
        virtual bool on_button_press_event(GdkEventButton* event);
        virtual void on_menu_popup_status();

        //
        virtual bool has_stopped();
        virtual void stop_work();
        void on_start_button_clicked();
        void on_stop_button_clicked();
        void on_quit_button_clicked();

    private:
        inputs<double,3> vars;
        BinoprGroup<3,double,Binopr> town;
        size_t iteration,iterations;
        mutable std::mutex m_Mutex;

        Glib::Dispatcher m_Dispatcher;
        std::thread* m_WorkerThread;

        bool m_shall_stop,m_has_stopped;
        void do_work();
        void load(const BinoprGroup<3,double,Binopr>& );

    };
}



#endif // MATH-APPLICATION_HH_INCLUDED
