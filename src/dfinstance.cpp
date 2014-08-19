/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <QtDebug>
#include <QMessageBox>
#if QT_VERSION >= 0x050000
# include <QtConcurrent>
#endif
#include "defines.h"
#include "dfinstance.h"
#include "dwarf.h"
#include "squad.h"
#include "word.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "dwarftherapist.h"
#include "memorysegment.h"
#include "truncatingfilelogger.h"
#include "mainwindow.h"
#include "limits"
#include "dwarfstats.h"
#include "QThreadPool"
#include "viewmanager.h"
#include "languages.h"
#include "reaction.h"
#include "races.h"
#include "itemweaponsubtype.h"
#include "fortressentity.h"
#include "material.h"
#include "plant.h"
#include "item.h"
#include "itemweapon.h"
#include "itemarmor.h"
#include "preference.h"

#ifdef Q_OS_WIN
#define LAYOUT_SUBDIR "windows"
#include "dfinstancewindows.h"
#else
#ifdef Q_OS_LINUX
#define LAYOUT_SUBDIR "linux"
#include "dfinstancelinux.h"
#else
#ifdef Q_OS_MAC
#define LAYOUT_SUBDIR "osx"
#include "dfinstanceosx.h"
#endif
#endif
#endif

DFInstance::DFInstance(QObject* parent)
    : QObject(parent)
    , m_pid(0)
    , m_stop_scan(false)
    , m_is_ok(true)
    , m_bytes_scanned(0)
    , m_layout(0)
    , m_attach_count(0)
    , m_heartbeat_timer(new QTimer(this))
    , m_memory_remap_timer(new QTimer(this))
    , m_scan_speed_timer(new QTimer(this))
    , m_dwarf_race_id(0)
    , m_dwarf_civ_id(0)
    , m_languages(0x0)
    , m_fortress(0x0)
    , m_fortress_name(tr("Embarking"))
    , m_fortress_name_translated("")
{
    connect(m_scan_speed_timer, SIGNAL(timeout()),
            SLOT(calculate_scan_rate()));
    connect(m_memory_remap_timer, SIGNAL(timeout()),
            SLOT(map_virtual_memory()));
    m_memory_remap_timer->start(20000); // 20 seconds
    // let subclasses start the heartbeat timer, since we don't want to be
    // checking before we're connected
    connect(m_heartbeat_timer, SIGNAL(timeout()), SLOT(heartbeat()));

    // We need to scan for memory layout files to get a list of DF versions this
    // DT version can talk to. Start by building up a list of search paths
    QDir working_dir = QDir::current();
    QStringList search_paths;
    search_paths << working_dir.path();

    QString subdir = LAYOUT_SUBDIR;
    search_paths << QString("%1/../share/memory_layouts/%2").arg(QCoreApplication::applicationDirPath(), subdir);

    TRACE << "Searching for MemoryLayout ini files in the following directories";
    foreach(QString path, search_paths) {
        TRACE<< path;
        QDir d(path);
        d.setNameFilters(QStringList() << "*.ini");
        d.setFilter(QDir::NoDotAndDotDot | QDir::Readable | QDir::Files);
        d.setSorting(QDir::Name | QDir::Reversed);
        QFileInfoList files = d.entryInfoList();
        foreach(QFileInfo info, files) {
            MemoryLayout *temp = new MemoryLayout(info.absoluteFilePath());
            if (temp && temp->is_valid()) {
                LOGI << "adding valid layout" << temp->game_version()
                     << temp->checksum();
                m_memory_layouts.insert(temp->checksum().toLower(), temp);
            }
        }
    }
    // if no memory layouts were found that's a critical error
    if (m_memory_layouts.size() < 1) {
        LOGE << "No valid memory layouts found in the following directories..."
             << QDir::searchPaths("memory_layouts");
        qApp->exit(ERROR_NO_VALID_LAYOUTS);
    }
}

DFInstance * DFInstance::newInstance(){
#ifdef Q_OS_WIN
    return new DFInstanceWindows();
#else
#ifdef Q_OS_MAC
    return new DFInstanceOSX();
#else
#ifdef Q_OS_LINUX
    return new DFInstanceLinux();
#endif
#endif
#endif
}

bool DFInstance::check_vector(const VIRTADDR start, const VIRTADDR end, const VIRTADDR addr){
    TRACE << "beginning vector enumeration at" << hex << addr;
    TRACE << "start of vector" << hex << start;
    TRACE << "end of vector" << hex << end;

    int entries = (end - start) / sizeof(VIRTADDR);
    TRACE << "there appears to be" << entries << "entries in this vector";

    bool is_acceptable_size = true;

    if (entries > 500000) {
        LOGW << "vector at" << hexify(addr) << "has over 500.000 entries! (" << entries << ")";
        is_acceptable_size = false;
    }else if (entries > 250000){
        LOGW << "vector at" << hexify(addr) << "has over 250.000 entries! (" << entries << ")";
    }else if (entries > 50000){
        LOGW << "vector at" << hexify(addr) << "has over 50.000 entries! (" << entries << ")";
    }else if (entries > 10000){
        LOGW << "vector at" << hexify(addr) << "has over 10.000 entries! (" << entries << ")";
    }

    if(!is_acceptable_size){
        LOGE << "vector at" << hexify(addr) << "was not read due to an unacceptable size! (" << entries << ")";
    }

    return is_acceptable_size;
}

DFInstance::~DFInstance() {
    //    LOGD << "DFInstance baseclass virtual dtor!";
    foreach(MemoryLayout *l, m_memory_layouts) {
        delete(l);
    }
    m_memory_layouts.clear();
    m_layout = 0;

    qDeleteAll(m_regions);
    m_regions.clear();

    delete m_languages;
    delete m_fortress;

    qDeleteAll(m_inorganics_vector);
    m_inorganics_vector.clear();
    qDeleteAll(m_base_materials);
    m_base_materials.clear();

    qDeleteAll(m_reactions);
    m_reactions.clear();
    qDeleteAll(m_races);
    m_races.clear();
    qDeleteAll(m_weapon_defs);
    m_weapon_defs.clear();
    m_ordered_weapon_defs.clear();
    qDeleteAll(m_plants_vector);
    m_plants_vector.clear();

    qDeleteAll(m_pref_counts);
    m_pref_counts.clear();

    m_thought_counts.clear();
}

BYTE DFInstance::read_byte(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(BYTE), out);
    return out.at(0);
}

WORD DFInstance::read_word(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(WORD), out);
    return decode_word(out);
}

VIRTADDR DFInstance::read_addr(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(VIRTADDR), out);
    return decode_dword(out);
}

qint16 DFInstance::read_short(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(qint16), out);
    return decode_short(out);
}

qint32 DFInstance::read_int(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(qint32), out);
    return decode_int(out);
}

QVector<VIRTADDR> DFInstance::scan_mem(const QByteArray &needle, const uint start_addr, const uint end_addr) {
    // progress reporting
    m_scan_speed_timer->start(500);
    m_memory_remap_timer->stop(); // don't remap segments while scanning
    int total_bytes = 0;
    m_bytes_scanned = 0; // for global timings
    int bytes_scanned = 0; // for progress calcs
    foreach(MemorySegment *seg, m_regions) {
        total_bytes += seg->size;
    }
    int report_every_n_bytes = total_bytes / 1000;
    emit scan_total_steps(1000);
    emit scan_progress(0);


    m_stop_scan = false;
    QVector<VIRTADDR> addresses; //! return value
    QByteArrayMatcher matcher(needle);

    int step_size = 0x1000;
    QByteArray buffer(step_size, 0);
    QByteArray back_buffer(step_size * 2, 0);

    QTime timer;
    timer.start();
    attach();
    foreach(MemorySegment *seg, m_regions) {
        int step = step_size;
        int steps = seg->size / step;
        if (seg->size % step)
            steps++;

        if( seg->end_addr < start_addr ) {
            continue;
        }

        if( seg->start_addr > end_addr ) {
            break;
        }

        for(VIRTADDR ptr = seg->start_addr; ptr < seg->end_addr; ptr += step) {

            if( ptr < start_addr ) {
                continue;
            }
            if( ptr > end_addr ) {
                m_stop_scan = true;
                break;
            }

            step = step_size;
            if (ptr + step > seg->end_addr) {
                step = seg->end_addr - ptr;
            }

            // move the last thing we read to the front of the back_buffer
            back_buffer.replace(0, step, buffer);

            // fill the main read buffer
            int bytes_read = read_raw(ptr, step, buffer);
            if (bytes_read < step && !seg->is_guarded) {
                if (m_layout->is_complete()) {
                    LOGW << "tried to read" << step << "bytes starting at" <<
                            hexify(ptr) << "but only got" << dec << bytes_read;
                }
                continue;
            }
            bytes_scanned += bytes_read;
            m_bytes_scanned += bytes_read;

            // put the main buffer on the end of the back_buffer
            back_buffer.replace(step, step, buffer);

            int idx = -1;
            forever {
                idx = matcher.indexIn(back_buffer, idx+1);
                if (idx == -1) {
                    break;
                } else {
                    VIRTADDR hit = ptr + idx - step;
                    if (!addresses.contains(hit)) {
                        // backbuffer may cause duplicate hits
                        addresses << hit;
                    }
                }
            }

            if (m_stop_scan)
                break;
            emit scan_progress(bytes_scanned / report_every_n_bytes);

        }
        DT->processEvents();
        if (m_stop_scan)
            break;
    }
    detach();
    m_memory_remap_timer->start(20000); // start the remapper again
    LOGD << QString("Scanned %L1MB in %L2ms").arg(bytes_scanned / 1024 * 1024)
            .arg(timer.elapsed());
    return addresses;
}

bool DFInstance::looks_like_vector_of_pointers(const VIRTADDR &addr) {
    int start = read_int(addr + 0x4);
    int end = read_int(addr + 0x8);
    int entries = (end - start) / sizeof(int);
    LOGD << "LOOKS LIKE VECTOR? unverified entries:" << entries;

    return start >=0 &&
            end >=0 &&
            end >= start &&
            (end-start) % 4 == 0 &&
            start % 4 == 0 &&
            end % 4 == 0 &&
            entries < 10000;

}

void DFInstance::load_game_data()
{
    map_virtual_memory();
    emit progress_message(tr("Loading languages"));
    if(m_languages){
        delete m_languages;
        m_languages = 0;
    }
    m_languages = Languages::get_languages(this);

    emit progress_message(tr("Loading reactions"));
    qDeleteAll(m_reactions);
    m_reactions.clear();
    load_reactions();

    emit progress_message(tr("Loading item and material lists"));
    qDeleteAll(m_plants_vector);
    m_plants_vector.clear();
    qDeleteAll(m_inorganics_vector);
    m_inorganics_vector.clear();
    qDeleteAll(m_base_materials);
    m_base_materials.clear();
    load_main_vectors();

    //load the currently played race before races and castes so we can load additional information for the current race being played
    VIRTADDR dwarf_race_index_addr = m_layout->address("dwarf_race_index");
    if (!is_valid_address(dwarf_race_index_addr)) {
        LOGW << "Active Memory Layout" << m_layout->filename() << "("
             << m_layout->game_version() << ")" << "contains an invalid"
             << "dwarf_race_index address. Either you are scanning a new "
             << "DF version or your config files are corrupted.";
        m_dwarf_race_id = -1;
    }else{
        LOGD << "dwarf race index" << hexify(dwarf_race_index_addr) << hexify(dwarf_race_index_addr) << "(UNCORRECTED)";
        // which race id is dwarven?
        m_dwarf_race_id = read_short(dwarf_race_index_addr);
        LOGD << "dwarf race:" << hexify(m_dwarf_race_id);
    }


    emit progress_message(tr("Loading races and castes"));
    qDeleteAll(m_races);
    m_races.clear();
    load_races_castes();

    emit progress_message(tr("Loading weapons"));
    qDeleteAll(m_weapon_defs);
    m_weapon_defs.clear();
    m_ordered_weapon_defs.clear();
    load_weapons();

    load_fortress_name();
}

QString DFInstance::get_language_word(VIRTADDR addr){
    return m_languages->language_word(addr);
}

QString DFInstance::get_translated_word(VIRTADDR addr){
    return m_languages->english_word(addr);
}

QVector<Dwarf*> DFInstance::load_dwarves() {
    map_virtual_memory();
    QVector<Dwarf*> dwarves;
    if (!m_is_ok) {
        LOGW << "not connected";
        detach();
        return dwarves;
    }

    // we're connected, make sure we have good addresses
    VIRTADDR creature_vector = m_layout->address("creature_vector");
    VIRTADDR active_creature_vector = m_layout->address("active_creature_vector");
    VIRTADDR current_year = m_layout->address("current_year");
    VIRTADDR current_year_tick = m_layout->address("cur_year_tick");
    m_cur_year_tick = read_int(current_year_tick);

    if (!is_valid_address(creature_vector) || !is_valid_address(active_creature_vector)) {
        LOGW << "Active Memory Layout" << m_layout->filename() << "("
             << m_layout->game_version() << ")" << "contains an invalid"
             << "creature_vector address. Either you are scanning a new "
             << "DF version or your config files are corrupted.";
        return dwarves;
    }

    //current race's offset was bad
    if (!DT->arena_mode && m_dwarf_race_id < 0){
        return dwarves;
    }    

    // both necessary addresses are valid, so let's try to read the creatures
    VIRTADDR dwarf_civ_idx_addr = m_layout->address("dwarf_civ_index");
    LOGD << "loading creatures from " << hexify(creature_vector);
    LOGD << "loading current year from" << hexify(current_year);

    emit progress_message(tr("Loading Units"));

    attach();
    m_dwarf_civ_id = read_int(dwarf_civ_idx_addr);
    LOGD << "civilization id:" << hexify(m_dwarf_civ_id);

    m_current_year = read_word(current_year);
    LOGI << "current year:" << m_current_year;
    m_cur_time = (int)m_current_year * 0x62700 + m_cur_year_tick;

    QVector<VIRTADDR> creatures_addrs = get_creatures();

    emit progress_range(0, creatures_addrs.size()-1);
    TRACE << "FOUND" << creatures_addrs.size() << "creatures";
    QTime t;
    t.start();
    if (!creatures_addrs.empty()) {
        Dwarf *d = 0;
        int progress_count = 0;

        foreach(VIRTADDR creature_addr, creatures_addrs) {
            d = Dwarf::get_dwarf(this, creature_addr);
            if(d){
                dwarves.append(d); //add animals as well so we can show them
                if(!d->is_animal()){
                    LOGI << "FOUND UNIT" << hexify(creature_addr) << d->nice_name();
                    m_actual_dwarves.append(d);

                    //never calculate roles for babies
                    //only calculate roles for children if they're shown and labor cheats are enabled
                    if(!d->is_baby()){
                        if(!d->is_child() || (DT->labor_cheats_allowed() && !DT->hide_non_adults())){
                            //dwarves_with_roles.append(d->id());
                            m_labor_capable_dwarves.append(d);
                        }
                    }

                } else {
                    LOGI << "FOUND BEAST" << hexify(creature_addr) << d->nice_name();
                }
            }
            emit progress_value(progress_count++);
        }
        LOGI << "read" << dwarves.count() << "units in" << t.elapsed() << "ms";

        m_enabled_labor_count.clear();
        qDeleteAll(m_pref_counts);
        m_pref_counts.clear();
        m_thought_counts.clear();

        t.restart();
        QFuture<void> f = QtConcurrent::run(this,&DFInstance::load_population_data);
        f.waitForFinished();
        LOGI << "loaded population data in" << t.elapsed() << "ms";

        t.restart();
        f = QtConcurrent::run(this,&DFInstance::load_role_ratings);
        f.waitForFinished();
        LOGI << "calculated roles in" << t.elapsed() << "ms";

        //calc_done();
        m_actual_dwarves.clear();
        m_labor_capable_dwarves.clear();

        DT->emit_labor_counts_updated();

    } else {
        // we lost the fort!
        m_is_ok = false;
    }
    detach();

    LOGI << "found" << dwarves.size() << "dwarves out of" << creatures_addrs.size() << "creatures";

    return dwarves;
}

void DFInstance::load_population_data(){
    int labor_count = 0;
    foreach(Dwarf *d, m_actual_dwarves){
        //load labor counts
        foreach(int key, d->get_labors().uniqueKeys()){
            if(d->labor_enabled(key)){
                if(m_enabled_labor_count.contains(key))
                    labor_count = m_enabled_labor_count.value(key)+1;
                else
                    labor_count = 1;
                m_enabled_labor_count.insert(key,labor_count);
            }
        }

        //load preference/thought totals, excluding babies/children according to settings
        if(d->is_adult() || (!d->is_adult() && !DT->hide_non_adults())){
            foreach(QString category_name, d->get_grouped_preferences().uniqueKeys()){
                for(int i = 0; i < d->get_grouped_preferences().value(category_name)->count(); i++){

                    QString pref = d->get_grouped_preferences().value(category_name)->at(i);
                    QString cat_name = category_name;
                    bool is_dislike = false;
                    //put liked and hated creatures together
                    if(category_name == Preference::get_pref_desc(HATE_CREATURE)){
                        cat_name = Preference::get_pref_desc(LIKE_CREATURE);
                        is_dislike = true;
                    }

                    QPair<QString,QString> key_pair;
                    key_pair.first = cat_name;
                    key_pair.second = pref;

                    pref_stat *p;
                    if(m_pref_counts.contains(key_pair))
                        p = m_pref_counts.take(key_pair);
                    else{
                        p = new pref_stat();
                    }

                    if(is_dislike){
                        p->names_dislikes.append(d->nice_name());
                    }else{
                        p->names_likes.append(d->nice_name());
                    }
                    p->pref_category = cat_name;

                    m_pref_counts.insert(key_pair,p);
                }
            }

            QHash<short,int> d_thoughts = d->get_thoughts();
            foreach(short id, d_thoughts.uniqueKeys()){
                int total_count = 0;
                int dwarf_count = 0;
                if(m_thought_counts.contains(id)){
                    total_count = m_thought_counts.value(id).first;
                    dwarf_count = m_thought_counts.value(id).second;
                }
                total_count += d_thoughts.value(id);
                dwarf_count++;
                m_thought_counts.insert(id,qMakePair(total_count,dwarf_count));
            }
        }
    }
}

void DFInstance::load_role_ratings(){
    if(m_labor_capable_dwarves.size() <= 0)
        return;

    QVector<double> attribute_values;
    QVector<double> attribute_raw_values;
    QVector<double> skill_values;
    QVector<double> trait_values;
    QVector<double> pref_values;

    foreach(Dwarf *d, m_labor_capable_dwarves){
        foreach(int id, GameDataReader::ptr()->get_attributes().keys()){
            attribute_values.append(d->get_attribute(id).get_balanced_value());
            attribute_raw_values.append(d->get_attribute(id).get_value());
        }

        foreach(int id, GameDataReader::ptr()->get_skills().keys()){
            skill_values.append(d->get_skill(id).get_balanced_level());
        }

        foreach(short val, d->get_traits()->values()){
            trait_values.append((double)val);
        }

        foreach(Role *r, GameDataReader::ptr()->get_roles().values()){
            if(r->prefs.count() > 0){
                foreach(double rating, d->get_role_pref_match_counts(r)){
                    pref_values.append(rating);
                }
            }
        }
    }

    QTime tr;
    tr.start();
    LOGD << "Role Trait Info:";
    DwarfStats::init_traits(trait_values);
    LOGD << "     - loaded trait role data in" << tr.elapsed() << "ms";

    LOGD << "Role Skills Info:";
    DwarfStats::init_skills(skill_values);
    LOGD << "     - loaded skill role data in" << tr.elapsed() << "ms";

    LOGD << "Role Attributes Info:";
    DwarfStats::init_attributes(attribute_values,attribute_raw_values);    
    LOGD << "     - loaded attribute role data in" << tr.elapsed() << "ms";

    LOGD << "Role Preferences Info:";
    DwarfStats::init_prefs(pref_values);
    LOGD << "     - loaded preference role data in" << tr.elapsed() << "ms";

    float role_rating_avg = 0;
    bool calc_role_avg = (DT->get_log_manager()->get_appender("core")->minimum_level() <= LL_DEBUG);

    QVector<double> all_role_ratings;
    foreach(Dwarf *d, m_labor_capable_dwarves){
        foreach(float rating, d->calc_role_ratings()){
            all_role_ratings.append(rating);
            if(calc_role_avg)
                role_rating_avg+=rating;
        }
        d->update_rating_list();
    }
    LOGD << "Role Drawing Info:";
    DwarfStats::init_roles(all_role_ratings);
    LOGD << "     - loaded role drawing data in" << tr.elapsed() << "ms";

    if(DT->get_log_manager()->get_appender("core")->minimum_level() <= LL_DEBUG){
        float max = 0;
        float min = 0;
        float median = 0;
        if(all_role_ratings.count() > 0){
            role_rating_avg /= all_role_ratings.count();
            QVector<double>::Iterator i_min = std::min_element(all_role_ratings.begin(),all_role_ratings.end());
            QVector<double>::Iterator i_max = std::max_element(all_role_ratings.begin(),all_role_ratings.end());
            max = *i_max;
            min = *i_min;
            median = 0;
            float n = (float)all_role_ratings.count() / 2.0f;
            if(all_role_ratings.count() % 2 == 0){
                std::nth_element(all_role_ratings.begin(),all_role_ratings.begin()+(int)n,all_role_ratings.end());
                median = 0.5*(all_role_ratings.at((int)n)+all_role_ratings.at((int)n-1));
            }else{
                std::nth_element(all_role_ratings.begin(),all_role_ratings.begin()+(int)n,all_role_ratings.end());
                median =  all_role_ratings.at((int)n);
            }
        }
        LOGD << "Overall Role Rating Stats";
        LOGD << "     - Min: " << min;
        LOGD << "     - Max: " << max;
        LOGD << "     - Median: " << median;
        LOGD << "     - Average: " << role_rating_avg;
    }

}


void DFInstance::load_reactions(){
    attach();
    //LOGI << "Reading reactions names...";
    VIRTADDR reactions_vector = m_layout->address("reactions_vector");
    if(m_layout->is_valid_address(reactions_vector)){        
        QVector<VIRTADDR> reactions = enumerate_vector(reactions_vector);
        //TRACE << "FOUND" << reactions.size() << "reactions";
        //emit progress_range(0, reactions.size()-1);
        if (!reactions.empty()) {
            foreach(VIRTADDR reaction_addr, reactions) {
                Reaction* r = Reaction::get_reaction(this, reaction_addr);
                m_reactions.insert(r->tag(), r);
                //emit progress_value(i++);
            }
        }
    }
    detach();
}

void DFInstance::load_main_vectors(){
    //material templates
    QVector<VIRTADDR> temps = enumerate_vector(m_layout->address("material_templates_vector"));
    foreach(VIRTADDR addr, temps){
        m_material_templates.insert(read_string(addr),addr);
    }

    //syndromes
    m_all_syndromes = enumerate_vector(m_layout->address("all_syndromes_vector"));

    //world.raws.itemdefs.
    QVector<VIRTADDR> weapons = enumerate_vector(m_layout->address("itemdef_weapons_vector"));
    m_itemdef_vectors.insert(WEAPON,weapons);
    QVector<VIRTADDR> traps = enumerate_vector(m_layout->address("itemdef_trap_vector"));
    m_itemdef_vectors.insert(TRAPCOMP,traps);
    QVector<VIRTADDR> toys = enumerate_vector(m_layout->address("itemdef_toy_vector"));
    m_itemdef_vectors.insert(TOY,toys);
    QVector<VIRTADDR> tools = enumerate_vector(m_layout->address("itemdef_tool_vector"));
    m_itemdef_vectors.insert(TOOL,tools);
    QVector<VIRTADDR> instruments = enumerate_vector(m_layout->address("itemdef_instrument_vector"));
    m_itemdef_vectors.insert(INSTRUMENT,instruments);
    QVector<VIRTADDR> armor = enumerate_vector(m_layout->address("itemdef_armor_vector"));
    m_itemdef_vectors.insert(ARMOR,armor);
    QVector<VIRTADDR> ammo = enumerate_vector(m_layout->address("itemdef_ammo_vector"));
    m_itemdef_vectors.insert(AMMO,ammo);
    QVector<VIRTADDR> siege_ammo = enumerate_vector(m_layout->address("itemdef_siegeammo_vector"));
    m_itemdef_vectors.insert(SIEGEAMMO,siege_ammo);
    QVector<VIRTADDR> gloves = enumerate_vector(m_layout->address("itemdef_glove_vector"));
    m_itemdef_vectors.insert(GLOVES,gloves);
    QVector<VIRTADDR> shoes = enumerate_vector(m_layout->address("itemdef_shoe_vector"));
    m_itemdef_vectors.insert(SHOES,shoes);
    QVector<VIRTADDR> shields = enumerate_vector(m_layout->address("itemdef_shield_vector"));
    m_itemdef_vectors.insert(SHIELD,shields);
    QVector<VIRTADDR> helms = enumerate_vector(m_layout->address("itemdef_helm_vector"));
    m_itemdef_vectors.insert(HELM,helms);
    QVector<VIRTADDR> pants = enumerate_vector(m_layout->address("itemdef_pant_vector"));
    m_itemdef_vectors.insert(PANTS,pants);
    QVector<VIRTADDR> food = enumerate_vector(m_layout->address("itemdef_food_vector"));
    m_itemdef_vectors.insert(FOOD,food);

    //load actual weapons and armor
    weapons = enumerate_vector(m_layout->address("weapons_vector"));
    m_items_vectors.insert(WEAPON,weapons);
    shields = enumerate_vector(m_layout->address("shields_vector"));
    m_items_vectors.insert(SHIELD,shields);

    pants = enumerate_vector(m_layout->address("pants_vector"));
    m_items_vectors.insert(PANTS,pants);
    armor = enumerate_vector(m_layout->address("armor_vector"));
    m_items_vectors.insert(ARMOR,armor);
    shoes = enumerate_vector(m_layout->address("shoes_vector"));
    m_items_vectors.insert(SHOES,shoes);
    helms = enumerate_vector(m_layout->address("helms_vector"));
    m_items_vectors.insert(HELM,helms);
    gloves = enumerate_vector(m_layout->address("gloves_vector"));
    m_items_vectors.insert(GLOVES,gloves);

    //load other equipment
    QVector<VIRTADDR> quivers = enumerate_vector(m_layout->address("quivers_vector"));
    m_items_vectors.insert(QUIVER,quivers);
    QVector<VIRTADDR> backpacks = enumerate_vector(m_layout->address("backpacks_vector"));
    m_items_vectors.insert(BACKPACK,backpacks);
    QVector<VIRTADDR> crutches = enumerate_vector(m_layout->address("crutches_vector"));
    m_items_vectors.insert(CRUTCH,crutches);
    QVector<VIRTADDR> flasks = enumerate_vector(m_layout->address("flasks_vector"));
    m_items_vectors.insert(FLASK,flasks);
    ammo = enumerate_vector(m_layout->address("ammo_vector"));
    m_items_vectors.insert(AMMO,ammo);

    //load artifacts
    QVector<VIRTADDR> artifacts = enumerate_vector(m_layout->address("artifacts_vector"));
    m_items_vectors.insert(ARTIFACTS,artifacts);

    m_color_vector = enumerate_vector(m_layout->address("colors_vector"));
    m_shape_vector = enumerate_vector(m_layout->address("shapes_vector"));

    VIRTADDR addr = m_layout->address("base_materials");
    int i = 0;
    for(i = 0; i < 256; i++){
        VIRTADDR mat_addr = read_addr(addr);
        if(mat_addr > 0){
            Material* m = Material::get_material(this, mat_addr, i, false, this);
            m_base_materials.append(m);
        }
        addr += 0x4;
    }

    //inorganics
    addr = m_layout->address("inorganics_vector");
    i = 0;
    foreach(VIRTADDR mat, enumerate_vector(addr)){
        //inorganic_raw.material
        Material* m = Material::get_material(this, mat, i, true, this);
        m_inorganics_vector.append(m);
        i++;
    }

    //plants
    addr = m_layout->address("plants_vector");
    i = 0;
    QVector<VIRTADDR> vec = enumerate_vector(addr);
    foreach(VIRTADDR plant, vec){
        Plant* p = Plant::get_plant(this, plant, i);
        m_plants_vector.append(p);
        i++;
    }
}

void DFInstance::load_weapons(){
    attach();
    QVector<VIRTADDR> weapons = m_itemdef_vectors.value(WEAPON);
    qDeleteAll(m_weapon_defs);
    m_weapon_defs.clear();
    if (!weapons.empty()) {
        foreach(VIRTADDR weapon_addr, weapons) {
            ItemWeaponSubtype* w = ItemWeaponSubtype::get_weapon(this, weapon_addr, this);
            m_weapon_defs.insert(w->name_plural(), w);
        }
    }

    m_ordered_weapon_defs.clear();

    QStringList weapon_names;
    foreach(QString key, m_weapon_defs.uniqueKeys()) {
        weapon_names << key;
    }

    qSort(weapon_names);
    foreach(QString name, weapon_names) {
        m_ordered_weapon_defs << QPair<QString, ItemWeaponSubtype *>(name, m_weapon_defs.value(name));
    }

    detach();
}

void DFInstance::load_races_castes(){
    attach();
    VIRTADDR races_vector_addr = m_layout->address("races_vector");
    QVector<VIRTADDR> races = enumerate_vector(races_vector_addr);
    int idx = 0;
    if (!races.empty()) {
        foreach(VIRTADDR race_addr, races) {
            m_races.append(Race::get_race(this, race_addr, idx));
            idx++;
        }
    }
    detach();
}

void DFInstance::load_fortress(){
    //load the fortress historical entity
    if(m_fortress){
        delete(m_fortress);
        m_fortress = 0;
    }
    VIRTADDR addr_fortress = m_layout->address("fortress_entity");
    if (!is_valid_address(addr_fortress)) {
        LOGW << "Active Memory Layout" << m_layout->filename() << "("
             << m_layout->game_version() << ")" << "contains an invalid"
             << "fortress identity address. Either you are scanning a new "
             << "DF version or your config files are corrupted.";
    }else{
        m_fortress = FortressEntity::get_entity(this,read_addr(addr_fortress));
    }
    if(m_fortress_name_translated.isEmpty())
        load_fortress_name();
}

void DFInstance::load_fortress_name(){
    //load the fortress name
    //fortress name is actually in the world data's site list
    //we can access a list of the currently active sites and read the name from there
    VIRTADDR world_data_addr = read_addr(m_layout->address("world_data"));
    QVector<VIRTADDR> sites = enumerate_vector(world_data_addr + m_layout->address("active_sites_vector"));
    foreach(VIRTADDR site, sites){
        short t = read_short(site + m_layout->address("world_site_type"));
        if(t==0){ //player fortress type
            m_fortress_name = get_language_word(site);
            m_fortress_name_translated = get_translated_word(site);
            break;
        }
    }
}

const QString DFInstance::fortress_name(){
    QString name = m_fortress_name;
    if(!m_fortress_name_translated.isEmpty())
        name.append(QString(", \"%1\"").arg(m_fortress_name_translated));
    return name;
}


QList<Squad *> DFInstance::load_squads(bool refreshing) {

    QList<Squad*> squads;
    if (!m_is_ok) {
        LOGW << "not connected";
        detach();
        return squads;
    }

    if(!refreshing){
        // we're connected, make sure we have good addresses
        m_squad_vector = m_layout->address("squad_vector");
        if(m_squad_vector == 0xFFFFFFFF) {
            LOGI << "Squads not supported for this version of Dwarf Fortress";
            return squads;
        }        

        if (!is_valid_address(m_squad_vector)) {
            LOGW << "Active Memory Layout" << m_layout->filename() << "("
                 << m_layout->game_version() << ")" << "contains an invalid"
                 << "squad_vector address. Either you are scanning a new "
                 << "DF version or your config files are corrupted.";
            return squads;
        }

        LOGD << "loading squads from " << hexify(m_squad_vector);

        emit progress_message(tr("Loading Squads"));
    }

    attach();

    QVector<VIRTADDR> squads_addr = enumerate_vector(m_squad_vector);
    LOGI << "FOUND" << squads_addr.size() << "squads";

    qDeleteAll(m_squads);
    m_squads.clear();

    if (!squads_addr.empty()) {
        if(!refreshing)
            emit progress_range(0, squads_addr.size()-1);

        int squad_count = 0;                
        foreach(VIRTADDR squad_addr, squads_addr) {
            int id = read_int(squad_addr + m_layout->squad_offset("id")); //check the id before loading the squad
            if(m_fortress->squad_is_active(id)){
                Squad *s = NULL;
                s = Squad::get_squad(id, this, squad_addr);
                if(s) {
                    LOGI << "FOUND ACTIVE SQUAD" << hexify(squad_addr) << s->name() << " member count: " << s->assigned_count() << " id: " << s->id();
                    if(m_fortress->squad_is_active(s->id())){
                        m_squads.push_front(s);
                    }
                }
            }

            if(!refreshing)
                emit progress_value(squad_count++);
        }
    }

    detach();
    //LOGI << "Found" << squads.size() << "squads out of" << m_current_creatures.size();
    return m_squads;
}

Squad* DFInstance::get_squad(int id){    
    foreach(Squad *s, m_squads){
        if(s->id() == id)
            return s;
    }
    return 0;
}


void DFInstance::heartbeat() {
    // simple read attempt that will fail if the DF game isn't running a fort, or isn't running at all
    // it would be nice to find a less cumbersome read, but for now at least we know this works
    if(get_creatures(false).size() < 1){
        // no game loaded, or process is gone
        emit connection_interrupted();
    }
}

QVector<VIRTADDR> DFInstance::get_creatures(bool report_progress){
    VIRTADDR active_units = m_layout->address("active_creature_vector");    
    VIRTADDR all_units = m_layout->address("creature_vector");    

    //first try the active unit list
    QVector<VIRTADDR> entries = enumerate_vector(active_units);
    if(entries.isEmpty()){
        if(report_progress){
            LOGI << "no active units (embark) using full unit list";
        }
        entries = enumerate_vector(all_units);
    }else{
        //there are active units, but are they ours?
        int civ_offset = m_layout->dwarf_offset("civ");
        foreach(VIRTADDR entry, entries){
            if(read_word(entry + civ_offset)==m_dwarf_civ_id){
                if(report_progress){
                    LOGI << "using active units";
                }
                return entries;
            }
        }
        if(report_progress){
            LOGI << "no active units with our civ (reclaim), using full unit list";
        }
        entries = enumerate_vector(all_units);
    }
    return entries;
}

bool DFInstance::is_valid_address(const VIRTADDR &addr) {
    bool valid = false;
    foreach(MemorySegment *seg, m_regions) {
        if (seg->contains(addr)) {
            valid = true;
            break;
        }
    }
    return valid;
}

QByteArray DFInstance::get_data(const VIRTADDR &addr, int size) {
    QByteArray ret_val(size, 0); // 0 filled to proper length
    int bytes_read = read_raw(addr, size, ret_val);
    if (bytes_read != size) {
        ret_val.clear();
    }
    return ret_val;
}

//! ahhh convenience
QString DFInstance::pprint(const VIRTADDR &addr, int size) {
    return pprint(get_data(addr, size), addr);
}

QString DFInstance::pprint(const QByteArray &ba, const VIRTADDR &start_addr) {
    QString out = "    ADDR   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | TEXT\n";
    out.append("------------------------------------------------------------------------\n");
    int lines = ba.size() / 16;
    if (ba.size() % 16)
        lines++;
    if (lines < 1)
        lines = 0;

    for(int i = 0; i < lines; ++i) {
        VIRTADDR offset = start_addr + i * 16;
        out.append(hexify(offset));
        out.append(" | ");
        for (int c = 0; c < 16; ++c) {
            out.append(ba.mid(i*16 + c, 1).toHex());
            out.append(" ");
        }
        out.append("| ");
        for (int c = 0; c < 16; ++c) {
            QByteArray tmp = ba.mid(i*16 + c, 1);
            if (tmp.at(0) == 0)
                out.append(".");
            else if (tmp.at(0) <= 126 && tmp.at(0) >= 32)
                out.append(tmp);
            else
                out.append(tmp.toHex());
        }
        //out.append(ba.mid(i*16, 16).toPercentEncoding());
        out.append("\n");
    }
    return out;
}

Word * DFInstance::read_dwarf_word(const VIRTADDR &addr) {
    Word * result = NULL;
    uint word_id = read_int(addr);
    if(word_id != 0xFFFFFFFF) {
        result = DT->get_word(word_id);
    }
    return result;
}

QString DFInstance::read_dwarf_name(const VIRTADDR &addr) {
    QString result = "The";

    //7 parts e.g.  ffffffff ffffffff 000006d4
    //      ffffffff ffffffff 000002b1 ffffffff

    //Unknown
    Word * word = read_dwarf_word(addr);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Unknown
    word = read_dwarf_word(addr + 0x04);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Verb
    word = read_dwarf_word(addr + 0x08);
    if(word) {
        result.append(" " + capitalize(word->adjective()));
    }

    //Unknown
    word = read_dwarf_word(addr + 0x0C);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Unknown
    word = read_dwarf_word(addr + 0x10);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Noun
    word = read_dwarf_word(addr + 0x14);
    bool singular = false;
    if(word) {
        if(word->plural_noun().isEmpty()) {
            result.append(" " + capitalize(word->noun()));
            singular = true;
        } else {
            result.append(" " + capitalize(word->plural_noun()));
        }
    }

    //of verb(noun)
    word = read_dwarf_word(addr + 0x18);
    if(word) {
        if( !word->verb().isEmpty() ) {
            if(singular) {
                result.append(" of " + capitalize(word->verb()));
            } else {
                result.append(" of " + capitalize(word->present_participle_verb()));
            }
        } else {
            if(singular) {
                result.append(" of " + capitalize(word->noun()));
            } else {
                result.append(" of " + capitalize(word->plural_noun()));
            }
        }
    }

    return result.trimmed();
}


QVector<VIRTADDR> DFInstance::find_vectors_in_range(const int &max_entries,
                                                    const VIRTADDR &start_address,
                                                    const int &range_length) {
    QByteArray data = get_data(start_address, range_length);
    QVector<VIRTADDR> vectors;
    VIRTADDR int1 = 0; // holds the start val
    VIRTADDR int2 = 0; // holds the end val

    for (int i = 0; i < range_length; i += 4) {
        memcpy(&int1, data.data() + i, 4);
        memcpy(&int2, data.data() + i + 4, 4);
        if (int2 >= int1 && is_valid_address(int1) && is_valid_address(int2)) {
            int bytes = int2 - int1;
            int entries = bytes / 4;
            if (entries > 0 && entries <= max_entries) {
                VIRTADDR vector_addr = start_address + i - VECTOR_POINTER_OFFSET;
                QVector<VIRTADDR> addrs = enumerate_vector(vector_addr);
                bool all_valid = true;
                foreach(VIRTADDR vec_entry, addrs) {
                    if (!is_valid_address(vec_entry)) {
                        all_valid = false;
                        break;
                    }
                }
                if (all_valid) {
                    vectors << vector_addr;
                }
            }
        }
    }
    return vectors;
}

QVector<VIRTADDR> DFInstance::find_vectors(int num_entries, int fuzz/* =0 */,
                                           int entry_size/* =4 */) {
    /*
    glibc++ does vectors like so...
    |4bytes      | 4bytes    | 4bytes
    START_ADDRESS|END_ADDRESS|END_ALLOCATOR

    MSVC++ does vectors like so...
    | 4bytes     | 4bytes      | 4 bytes   | 4bytes
    ALLOCATOR    |START_ADDRESS|END_ADDRESS|END_ALLOCATOR
    */
    m_stop_scan = false; //! if ever set true, bail from the inner loop
    QVector<VIRTADDR> vectors; //! return value collection of vectors found
    VIRTADDR int1 = 0; // holds the start val
    VIRTADDR int2 = 0; // holds the end val

    // progress reporting
    m_scan_speed_timer->start(500);
    m_memory_remap_timer->stop(); // don't remap segments while scanning
    int total_bytes = 0;
    m_bytes_scanned = 0; // for global timings
    int bytes_scanned = 0; // for progress calcs
    foreach(MemorySegment *seg, m_regions) {
        total_bytes += seg->size;
    }
    int report_every_n_bytes = total_bytes / 1000;
    emit scan_total_steps(1000);
    emit scan_progress(0);

    int scan_step_size = 0x10000;
    QByteArray buffer(scan_step_size, '\0');
    QTime timer;
    timer.start();
    attach();
    foreach(MemorySegment *seg, m_regions) {
        //TRACE << "SCANNING REGION" << hex << seg->start_addr << "-"
        //        << seg->end_addr << "BYTES:" << dec << seg->size;
        if ((int)seg->size <= scan_step_size) {
            scan_step_size = seg->size;
        }
        int scan_steps = seg->size / scan_step_size;
        if (seg->size % scan_step_size) {
            scan_steps++;
        }
        VIRTADDR addr = 0; // the ptr we will read from
        for(int step = 0; step < scan_steps; ++step) {
            addr = seg->start_addr + (scan_step_size * step);
            //LOGD << "starting scan for vectors at" << hex << addr << "step"
            //        << dec << step << "of" << scan_steps;
            int bytes_read = read_raw(addr, scan_step_size, buffer);
            if (bytes_read < scan_step_size) {
                continue;
            }
            for(int offset = 0; offset < scan_step_size; offset += entry_size) {
                int1 = decode_int(buffer.mid(offset, entry_size));
                int2 = decode_int(buffer.mid(offset + entry_size, entry_size));
                if (int1 && int2 && int2 >= int1
                        && int1 % 4 == 0
                        && int2 % 4 == 0
                        //&& is_valid_address(int1)
                        //&& is_valid_address(int2)
                        ) {
                    int bytes = int2 - int1;
                    int entries = bytes / entry_size;
                    int diff = entries - num_entries;
                    if (qAbs(diff) <= fuzz) {
                        VIRTADDR vector_addr = addr + offset -
                                VECTOR_POINTER_OFFSET;
                        QVector<VIRTADDR> addrs = enumerate_vector(vector_addr);
                        diff = addrs.size() - num_entries;
                        if (qAbs(diff) <= fuzz) {
                            vectors << vector_addr;
                        }
                    }
                }
                m_bytes_scanned += entry_size;
                bytes_scanned += entry_size;
                if (m_stop_scan)
                    break;
            }
            emit scan_progress(bytes_scanned / report_every_n_bytes);
            DT->processEvents();
            if (m_stop_scan)
                break;
        }
    }
    detach();
    m_memory_remap_timer->start(20000); // start the remapper again
    m_scan_speed_timer->stop();
    LOGD << QString("Scanned %L1MB in %L2ms").arg(bytes_scanned / 1024 * 1024)
            .arg(timer.elapsed());
    emit scan_progress(100);
    return vectors;
}

QVector<VIRTADDR> DFInstance::find_vectors_ext(int num_entries, const char op,
                                               const uint start_addr, const uint end_addr, int entry_size/* =4 */) {
    /*
    glibc++ does vectors like so...
    |4bytes      | 4bytes    | 4bytes
    START_ADDRESS|END_ADDRESS|END_ALLOCATOR

    MSVC++ does vectors like so...
    | 4bytes     | 4bytes      | 4 bytes   | 4bytes
    ALLOCATOR    |START_ADDRESS|END_ADDRESS|END_ALLOCATOR
    */
    m_stop_scan = false; //! if ever set true, bail from the inner loop
    QVector<VIRTADDR> vectors; //! return value collection of vectors found
    VIRTADDR int1 = 0; // holds the start val
    VIRTADDR int2 = 0; // holds the end val

    // progress reporting
    m_scan_speed_timer->start(500);
    m_memory_remap_timer->stop(); // don't remap segments while scanning
    int total_bytes = 0;
    m_bytes_scanned = 0; // for global timings
    int bytes_scanned = 0; // for progress calcs
    foreach(MemorySegment *seg, m_regions) {
        total_bytes += seg->size;
    }
    int report_every_n_bytes = total_bytes / 1000;
    emit scan_total_steps(1000);
    emit scan_progress(0);

    int scan_step_size = 0x10000;
    QByteArray buffer(scan_step_size, '\0');
    QTime timer;
    timer.start();
    attach();
    foreach(MemorySegment *seg, m_regions) {
        //TRACE << "SCANNING REGION" << hex << seg->start_addr << "-"
        //        << seg->end_addr << "BYTES:" << dec << seg->size;
        if ((int)seg->size <= scan_step_size) {
            scan_step_size = seg->size;
        }
        int scan_steps = seg->size / scan_step_size;
        if (seg->size % scan_step_size) {
            scan_steps++;
        }

        if( seg->end_addr < start_addr ) {
            continue;
        }

        if( seg->start_addr > end_addr ) {
            break;
        }

        VIRTADDR addr = 0; // the ptr we will read from
        for(int step = 0; step < scan_steps; ++step) {
            addr = seg->start_addr + (scan_step_size * step);
            //LOGD << "starting scan for vectors at" << hex << addr << "step"
            //        << dec << step << "of" << scan_steps;
            int bytes_read = read_raw(addr, scan_step_size, buffer);
            if (bytes_read < scan_step_size) {
                continue;
            }

            for(int offset = 0; offset < scan_step_size; offset += entry_size) {
                VIRTADDR vector_addr = addr + offset - VECTOR_POINTER_OFFSET;

                if( vector_addr < start_addr ) {
                    continue;
                }

                if( vector_addr > end_addr ) {
                    m_stop_scan = true;
                    break;
                }

                int1 = decode_int(buffer.mid(offset, entry_size));
                int2 = decode_int(buffer.mid(offset + entry_size, entry_size));
                if (int1 && int2 && int2 >= int1
                        && int1 % 4 == 0
                        && int2 % 4 == 0
                        //&& is_valid_address(int1)
                        //&& is_valid_address(int2)
                        ) {
                    int bytes = int2 - int1;
                    int entries = bytes / entry_size;
                    if (entries > 0 && entries < 1000) {
                        QVector<VIRTADDR> addrs = enumerate_vector(vector_addr);
                        if( (op == '=' && addrs.size() == num_entries)
                                || (op == '<' && addrs.size() < num_entries)
                                || (op == '>' && addrs.size() > num_entries) ) {
                            vectors << vector_addr;
                        }
                    }
                }
                m_bytes_scanned += entry_size;
                bytes_scanned += entry_size;
                if (m_stop_scan)
                    break;
            }
            emit scan_progress(bytes_scanned / report_every_n_bytes);
            DT->processEvents();
            if (m_stop_scan)
                break;
        }
    }
    detach();
    m_memory_remap_timer->start(20000); // start the remapper again
    m_scan_speed_timer->stop();
    LOGD << QString("Scanned %L1MB in %L2ms").arg(bytes_scanned / 1024 * 1024)
            .arg(timer.elapsed());
    emit scan_progress(100);
    return vectors;
}

QVector<VIRTADDR> DFInstance::find_vectors(int num_entries, const QVector<VIRTADDR> & search_set,
                                           int fuzz/* =0 */, int entry_size/* =4 */) {

    m_stop_scan = false; //! if ever set true, bail from the inner loop
    QVector<VIRTADDR> vectors; //! return value collection of vectors found

    // progress reporting
    m_scan_speed_timer->start(500);
    m_memory_remap_timer->stop(); // don't remap segments while scanning

    int total_vectors = vectors.size();
    m_bytes_scanned = 0; // for global timings
    int vectors_scanned = 0; // for progress calcs

    emit scan_total_steps(total_vectors);
    emit scan_progress(0);

    QTime timer;
    timer.start();
    attach();

    int vector_size = 8 + VECTOR_POINTER_OFFSET;
    QByteArray buffer(vector_size, '\0');

    foreach(VIRTADDR addr, search_set) {
        int bytes_read = read_raw(addr, vector_size, buffer);
        if (bytes_read < vector_size) {
            continue;
        }

        VIRTADDR int1 = 0; // holds the start val
        VIRTADDR int2 = 0; // holds the end val
        int1 = decode_int(buffer.mid(VECTOR_POINTER_OFFSET, sizeof(VIRTADDR)));
        int2 = decode_int(buffer.mid(VECTOR_POINTER_OFFSET+ sizeof(VIRTADDR), sizeof(VIRTADDR)));

        if (int1 && int2 && int2 >= int1
                && int1 % 4 == 0
                && int2 % 4 == 0) {

            int bytes = int2 - int1;
            int entries = bytes / entry_size;
            int diff = entries - num_entries;
            if (qAbs(diff) <= fuzz) {
                QVector<VIRTADDR> addrs = enumerate_vector(addr);
                diff = addrs.size() - num_entries;
                if (qAbs(diff) <= fuzz) {
                    vectors << addr;
                }
            }
        }

        vectors_scanned++;

        if(vectors_scanned % 100 == 0) {
            emit scan_progress(vectors_scanned);
            DT->processEvents();
        }

        if (m_stop_scan)
            break;
    }


    detach();
    m_memory_remap_timer->start(20000); // start the remapper again
    m_scan_speed_timer->stop();
    LOGD << QString("Scanned %L1 vectors in %L2ms").arg(vectors_scanned)
            .arg(timer.elapsed());
    emit scan_progress(100);
    return vectors;
}


MemoryLayout *DFInstance::get_memory_layout(QString checksum, bool) {
    checksum = checksum.toLower();
    LOGI << "DF's checksum is:" << checksum;

    MemoryLayout *ret_val = NULL;
    ret_val = m_memory_layouts.value(checksum, NULL);
    m_is_ok = ret_val != NULL && ret_val->is_valid();

    if(!m_is_ok) {
        LOGI << "Could not find layout for checksum" << checksum;
        DT->get_main_window()->check_for_layout(checksum);
    }

    if (m_is_ok) {
        LOGI << "Detected Dwarf Fortress version"
             << ret_val->game_version() << "using MemoryLayout from"
             << ret_val->filename();
    }

    return ret_val;
}

bool DFInstance::add_new_layout(const QString & version, QFile & file) {
    QString newFileName = version;
    newFileName.replace("(", "").replace(")", "").replace(" ", "_");
    newFileName +=  ".ini";

    QFileInfo newFile(QDir(QString("share/memory_layouts/%1").arg(LAYOUT_SUBDIR)), newFileName);
    newFileName = newFile.absoluteFilePath();

    if(!file.exists()) {
        LOGW << "Layout file" << file.fileName() << "does not exist!";
        return false;
    }

    LOGI << "Copying: " << file.fileName() << " to " << newFileName;
    if(!file.copy(newFileName)) {
        LOGW << "Error renaming layout file!";
        return false;
    }

    MemoryLayout *temp = new MemoryLayout(newFileName);
    if (temp && temp->is_valid()) {
        LOGI << "adding valid layout" << temp->game_version() << temp->checksum();
        m_memory_layouts.insert(temp->checksum().toLower(), temp);
    }
    return true;
}

void DFInstance::layout_not_found(const QString & checksum) {
    QString supported_vers;

    // TODO: Replace this with a rich dialog at some point that
    // is also accessible from the help menu. For now, remove the
    // extra path information as the dialog is getting way too big.
    // And make a half-ass attempt at sorting
    QList<MemoryLayout *> layouts = m_memory_layouts.values();
    qSort(layouts);

    foreach(MemoryLayout * l, layouts) {
        supported_vers.append(
                    QString("<li><b>%1</b>(<font color=\"#444444\">%2"
                            "</font>)</li>")
                    .arg(l->game_version())
                    .arg(l->checksum()));
    }

    QMessageBox *mb = new QMessageBox(qApp->activeWindow());
    mb->setIcon(QMessageBox::Critical);
    mb->setWindowTitle(tr("Unidentified Game Version"));
    mb->setText(tr("I'm sorry but I don't know how to talk to this "
                   "version of Dwarf Fortress! (checksum:%1)<br><br> <b>Supported "
                   "Versions:</b><ul>%2</ul>").arg(checksum).arg(supported_vers));

    /*
    mb->setDetailedText(tr("Failed to locate a memory layout file for "
        "Dwarf Fortress exectutable with checksum '%1'").arg(checksum));
    */
    mb->exec();
    LOGE << tr("unable to identify version from checksum:") << checksum;
}

void DFInstance::calculate_scan_rate() {
    float rate = (m_bytes_scanned / 1024.0f / 1024.0f) /
            (m_scan_speed_timer->interval() / 1000.0f);
    QString msg = QString("%L1MB/s").arg(rate);
    emit scan_message(msg);
    m_bytes_scanned = 0;
}


VIRTADDR DFInstance::find_historical_figure(int hist_id){
    if(m_hist_figures.count() <= 0)
        load_hist_figures();

    return m_hist_figures.value(hist_id,0);
}

void DFInstance::load_hist_figures(){
    QVector<VIRTADDR> hist_figs = enumerate_vector(m_layout->address("historical_figures_vector"));
    int hist_id = 0;
    //it may be possible to filter this list by only the current race.
    //need to test whether or not vampires will steal names from other races
    //this may also break nicknames on kings/queens if they belong to a different race...
    foreach(VIRTADDR fig, hist_figs){
        //if(read_int(fig + 0x0002) == dwarf_race_id() || read_int(fig + 0x00a0) == dwarf_civ_id()){
        hist_id = read_int(fig + m_layout->hist_figure_offset("id"));
        m_hist_figures.insert(hist_id,fig);
        //}
    }
}

VIRTADDR DFInstance::find_fake_identity(int hist_id){
    VIRTADDR fig = find_historical_figure(hist_id);
    if(fig){
        VIRTADDR fig_info = read_addr(fig + m_layout->hist_figure_offset("hist_fig_info"));
        VIRTADDR rep_info = read_addr(fig_info + m_layout->hist_figure_offset("reputation"));
        if(rep_info != 0){
            int cur_ident = read_int(rep_info + m_layout->hist_figure_offset("current_ident"));
            if(m_fake_identities.count() == 0) //lazy load fake identities
                m_fake_identities = enumerate_vector(m_layout->address("fake_identities_vector"));
            foreach(VIRTADDR ident, m_fake_identities){
                int fake_id = read_int(ident);
                if(fake_id==cur_ident){
                    return ident;
                }
            }
        }
    }
    return 0;
}

QVector<VIRTADDR> DFInstance::get_item_vector(ITEM_TYPE i){
    if(m_itemdef_vectors.contains(i))
        return m_itemdef_vectors.value(i);
    else
        return m_itemdef_vectors.value(NONE);
}

QString DFInstance::get_preference_item_name(int index, int subtype){
    ITEM_TYPE itype = static_cast<ITEM_TYPE>(index);

    QVector<VIRTADDR> items = get_item_vector(itype);
    if(!items.empty() && (subtype >=0 && subtype < items.count())){
        QString name = read_string(items.at(subtype) + m_layout->item_subtype_offset("name_plural"));
        if(itype==TRAPCOMP || itype==WEAPON){
            name.prepend(" ").prepend(read_string(items.at(subtype) + m_layout->item_subtype_offset("adjective")));
        }else if(itype==ARMOR || itype==PANTS){
            name.prepend(" ").prepend(read_string(items.at(subtype) + m_layout->armor_subtype_offset("mat_name")));
        }
        return name.trimmed();
    }
    else{
        return Item::get_item_name_plural(itype);
    }
}

VIRTADDR DFInstance::get_item_address(ITEM_TYPE itype, int item_id){
    if(m_mapped_items.value(itype).count() <= 0)
        index_item_vector(itype);
    if(m_mapped_items.contains(itype)){
        if(m_mapped_items.value(itype).contains(item_id))
            return m_mapped_items.value(itype).value(item_id);
    }
    return 0;
}

QString DFInstance::get_item_name(ITEM_TYPE itype, int item_id){
    if(m_mapped_items.value(itype).count() <= 0)
        index_item_vector(itype);

    if(itype == ARTIFACTS){
        VIRTADDR addr = m_mapped_items.value(itype).value(item_id);
        QString name = read_dwarf_name(addr+0x4);
        //name = read_dwarf_word(addr+0x4);
        name = get_language_word(addr+0x4);
        return name;
    }else{
        if(itype == WEAPON){
            ItemWeapon w = ItemWeapon(this,m_mapped_items.value(itype).value(item_id));
            return w.display_name(true);
        }else if(Item::is_armor_type(itype)){
            ItemArmor a = ItemArmor(this,m_mapped_items.value(itype).value(item_id));
            return a.display_name(true);
        }else{
            Item i = Item(this,m_mapped_items.value(itype).value(item_id));
            return i.display_name(true);
        }
        return tr("Specific ") + capitalizeEach(get_item_name(itype,-1,-1,-1));
    }
}

void DFInstance::index_item_vector(ITEM_TYPE itype){
    QHash<int,VIRTADDR> items;
    int offset = 0x18;
    if(itype == ARTIFACTS)
        offset = 0x0;
    foreach(VIRTADDR addr, m_items_vectors.value(itype)){
        items.insert(read_int(addr+offset),addr);
    }
    m_mapped_items.insert(itype,items);
}

QString DFInstance::get_item_name(ITEM_TYPE itype, int subtype, short mat_type, int mat_index, MATERIAL_CLASS mat_class){
    QVector<VIRTADDR> items = get_item_vector(itype);
    QStringList name_parts;
    QString mat_name = "";

    if(mat_class >= 0){
        mat_name = Material::get_mat_class_desc(mat_class);
    }else{
        if(mat_index >= 0 || mat_type >= 0)
            mat_name = find_material_name(mat_index,mat_type,itype);
    }
    name_parts.append(mat_name);

    if(!items.empty() && (subtype >=0 && subtype < items.count())){
        name_parts.append(read_string(items.at(subtype) + 0x3c));
        if(itype==TRAPCOMP || itype==WEAPON)//{
            name_parts.append(read_string(items.at(subtype) + m_layout->armor_subtype_offset("adjective")));
    }
    else{
        name_parts.append(Item::get_item_name(itype));
    }
    return name_parts.join(" ").trimmed();
}

QString DFInstance::get_color(int index){
    if(index < m_color_vector.count())
        return read_string(m_color_vector.at(index) + m_layout->descriptor_offset("color_name"));
    else
        return "unknown color";
}

QString DFInstance::get_shape(int index){
    if(index < m_shape_vector.count())
        return read_string(m_shape_vector.at(index) + m_layout->descriptor_offset("shape_name_plural"));
    else
        return "unknown shape";
}

Material *DFInstance::get_inorganic_material(int index){    
    if(index < m_inorganics_vector.count())
        return m_inorganics_vector.at(index);
    else
        return new Material(this);
}

Plant *DFInstance::get_plant(int index){
    if(index < m_plants_vector.count())
        return m_plants_vector.at(index);
    else
        return new Plant(this);
}

Material *DFInstance::get_raw_material(int index){
    if(index >= 0 && index < m_base_materials.size())
        return m_base_materials.at(index);
    else
        return new Material(this);
}

QString DFInstance::find_material_name(int mat_index, short mat_type, ITEM_TYPE itype){
    Material *m = find_material(mat_index, mat_type);
    QString name = "";

    if(!m)
        return name;

    if(mat_index < 0){
        name = m->get_material_name(SOLID);
    }
    else if(mat_type == 0){
        name = m->get_material_name(SOLID);
    }
    else if(mat_type < 19){
        name = m->get_material_name(SOLID);
    }
    else if(mat_type < 219){
        Race* r = get_race(mat_index);
        if(r)
        {
            if(itype == DRINK || itype == LIQUID_MISC)
                name = m->get_material_name(LIQUID);
            else if(itype == CHEESE)
                name = m->get_material_name(SOLID);
            else
            {
                name = r->name().toLower();
                name.append(" ");
                name.append(m->get_material_name(SOLID));
            }
        }
    }
    else if(mat_type < 419)
    {
        VIRTADDR hist_figure = find_historical_figure(mat_index);
        if(hist_figure){
            Race *r = get_race(read_int(hist_figure + m_layout->hist_figure_offset("hist_race")));
            QString fig_name = read_string(hist_figure + m_layout->hist_figure_offset("hist_name"));
            if(r){
                name = fig_name.append("'s ");
                name.append(m->get_material_name(LIQUID));
            }
        }
    }
    else if(mat_type < 619){
        Plant *p = get_plant(mat_index);
        if(p){
            name = p->name();

            if(itype==LEAVES_FRUIT)
                name = p->leaf_plural();
            else if(itype==SEEDS)
                name = p->seed_plural();
            else if(itype==PLANT)
                name = p->name_plural();            

            //specific plant material
            if(m){
                if(itype == DRINK || itype == LIQUID_MISC)
                    name = m->get_material_name(LIQUID);
                else if(itype == POWDER_MISC || itype == CHEESE)
                    name = m->get_material_name(POWDER);
                else if(Item::is_armor_type(itype)){
                    //don't include the 'fabric' part if it's a armor (item?) ie. pig tail fiber coat, not pig tail fiber fabric coat
                    name = p->name().append(" ").append(m->get_material_name(SOLID));
                }else if(m->flags().has_flag(LEAF_MAT) && m->flags().has_flag(STRUCTURAL_PLANT_MAT)){
                    name = p->name().append(" ").append(m->get_material_name(GENERIC));//fruit
                }else if(itype == NONE || m->flags().has_flag(IS_WOOD)){
                    name.append(" ").append(m->get_material_name(GENERIC));                    
                }

            }
        }
    }
    m = 0;
    return name.toLower().trimmed();
}

Material *DFInstance::find_material(int mat_index, short mat_type){
    int index = 0;
    Material *m = new Material(this);

    if(mat_index < 0){
        m = get_raw_material(mat_type);
    }
    else if(mat_type == 0){
        m = get_inorganic_material(mat_index);
    }
    else if(mat_type < 19){
        m = get_raw_material(mat_type);
    }
    else if(mat_type < 219){
        Race* r = get_race(mat_index);
        if(r)
        {
            index = mat_type - 19; //base material types
            m = r->get_creature_material(index);
            r = 0;
        }
    }
    else if(mat_type < 419)
    {
        VIRTADDR hist_figure = find_historical_figure(mat_index);
        if(hist_figure){
            Race *r = get_race(read_int(hist_figure + m_layout->hist_figure_offset("hist_race")));
            if(r){
                m = r->get_creature_material(mat_type-219);
                r = 0;
            }
        }
    }
    else if(mat_type < 619){
        Plant *p = get_plant(mat_index);
        index = mat_type -419;
        if(p)
            if(index < p->material_count()){
                m = p->get_plant_material(index);
            }
        p = 0;
    }

    return m;
}


VIRTADDR DFInstance::get_syndrome(int idx){
    if(idx >= 0 && idx < m_all_syndromes.size()){
        {return m_all_syndromes.at(idx);}
    }else{
        return -1;
    }
}
