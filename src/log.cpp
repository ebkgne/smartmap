#include "log.hpp"
#include "gui.hpp"
#include "editor.hpp"
#include <cstdint>

Log::~Log() {}
Log::Log() {

    File::write("build/logs/logs.txt", "");

    plog::init(plog::verbose, &appender);

}

Log::Appender::Appender() : plog::IAppender() { } // useless !

void Log::Appender::write(const plog::Record& record) {

    if (record.getSeverity() <= plog::Severity::error){
    
        cmd = true;
        
    }


    if (list.size()>3000) {
        list.pop_front();
    }

    static uint32_t uid = 0;
    list.push_back(Message{plog::FuncMessageFormatter::format(record), record.getSeverity(), record.getTime(), uid++ });

    // post first line
    std::ifstream ifile(File::loc()+"build/logs/logs.txt");
    std::stringstream buffer;
    buffer << plog::FuncMessageFormatter::format(record);
    if (cmd) 
        if (record.getSeverity() <= max_severity) {
            std::string cout = plog::FuncMessageFormatter::format(record);
            if (cout.length() > 200) 
                cout = cout.substr(0,200) + "...\n";

            // if (engine.running && engine.gui_v){
            //     bool already = false;
            //     auto debug = engine.tree->find("main");
            //     for (auto x : engine.gui_v->editors) 
            //         if (x->selected == debug) {
            //             already = true;
            //             break;
            //     }
            //     if (!already)
            //         engine.gui_v->editors.push_back(std::make_shared< EditorWidget>(engine.gui_v, engine.tree->find("main")));
        
            // }else
                std::cout << cout;
    }
    
    buffer << ifile.rdbuf();
    ifile.close();

    std::ofstream file(File::loc()+"build/logs/logs.txt");
    file << buffer.rdbuf();
    file.close();

}
