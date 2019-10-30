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

#include <QMessageBox>
#include "scriptdialog.h"
#include "ui_scriptdialog.h"
#include "gamedatareader.h"
#include "labor.h"
#include "trait.h"
#include "dwarftherapist.h"
#include "unithealth.h"
#include "healthcategory.h"
#include "healthinfo.h"
#include "item.h"
#include "dwarf.h"
#include "adaptivecolorfactory.h"

#include <QJSEngine>

// CSS stylesheet used for the documentation QTextEdits
static const char *doc_style = R"***(
h1 {
    font-size: x-large;
}

h2 {
    font-size: large;
}

p {
}

.comment {
    font-style: italic;
}

pre.example {
    color: %1;
    margin: 0;
}

span.type {
    color: %2;
}

span.identifier {
    font-weight: bold;
}

span.arg {
    color: %3;
}
)***";

static const char *general_doc = R"***(
<h1>Filter Scripts</h1>
<p>This editor allows the creation of simple scripts for the filtering of dwarfs in the main list. By creating Javascript-like statements, you can tell Dwarf Therapist which dwarfs should be shown.</p>
<p>The engine handles looping for you. All you need to provide is the boolean tests a dwarf must pass to be displayed. Each dwarf object is provided to the script as the variable "d". Different skills, labors, attributes, traits etc... can be accessed as members of that dwarf. For instance, d.rage is a dwarf's numeric trait score for "Rage" attribute.</p>

<h2>Some Examples</h2>
<p class="comment">Show only citizens with mining enabled. 0 is the id of "Mining" in the game_data.ini file that came with Dwarf Therapist</p>
<pre class="example">d.labor_enabled(0)</pre>

<p class="comment">Show citizens with either mining or stone detailing enabled but no citizens with bowmaking.</p>
<pre class="example">d.labor_enabled(0) || d.labor_enabled(12)) &amp;&amp; !d.labor_enabled(66)</pre>

<p class="comment">Show citizens without any labors assigned, excluding hauling labors</p>
<pre class="example">d.total_assigned_labors(false) &lt;= 0 &amp;&amp; d.squad_id() &lt; 0</pre>

<p class="comment">Show citizens without noble positions and with either no material preferences, or no item preferences</p>
<pre class="example">d.noble_position() == "" &amp;&amp; (!d.has_preference('','Items') &amp;&amp; !d.has_preference('','Materials')) || !d.has_preference('','Items') || !d.has_preference('','Materials')</pre>

<h2>General Unit Methods</h2>
<p>The following methods can be called on the "d" object to get more information about a unit.</p>
<p class="method"><span class="type">string</span> <span class="identifier">nice_name</span>()</p>
<p class="method"><span class="type">string</span> <span class="identifier">nickname</span>()</p>
<p class="method"><span class="type">bool</span>  <span class="identifier">name_matches</span>(<span class="type">string</span> <span class="arg">val</span>)
    <span class="comment">**case insensitive search in the unit's full name</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_gender</span>()
    <span class="comment">**unknown = -1, female = 0, male = 1</span></p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_male</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_female</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">get_orientation</span>()
    <span class="comment">**asexual=0, bisexual=1, homosexual=2, heterosexual=3</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">same_sex_commitment</span>()
    <span class="comment">**uninterested=0, lover=1, marriage=2</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">other_sex_commitment</span>()
    <span class="comment">**uninterested=0, lover=1, marriage=2</span></p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_baby</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_child</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_adult</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_citizen</span>()
    <span class="comment">**not a visitor/guest ie. can have labors enabled</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">current_job_id()</span></p>
<p class="method"><span class="type">string</span> <span class="identifier">noble_position</span>()
    <span class="comment">**returns a comma separated list of the unit's noble positions</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_raw_happiness</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">body_size</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">body_size_base</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">migration_wave</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_cursed</span>()</p>
<p class="method"><span class="type">string</span> <span class="identifier">curse_name</span>()
    <span class="comment">**if the unit is cursed, returns their true name</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_age</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">get_birth_time</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">get_birth_year)</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">born_in_fortress</span>()</p>
<p class="method"><span class="type">short</span> <span class="identifier">get_caste_id</span>()</p>
<p class="method"><span class="type">string</span> <span class="identifier">get_caste_tag</span>()</p>
<p class="method"><span class="type">string</span> <span class="identifier">get_caste_name</span>()</p>
<p class="method"><span class="type">string</span> <span class="identifier">get_caste_desc</span>()</p>
<p class="method"><span class="type">short</span> <span class="identifier">get_race_id</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">has_preference</span>(<span class="type">string</span> <span class="arg">pref_name</span>, <span class="type">string</span> <span class="arg">pref_category</span>)
    <span class="comment">**leave pref_category blank to search in all categories. eg. has_preference("copper","")</span></p>
<p class="method"><span class="type">bool</span> <span class="identifier">has_thought</span>(<span class="type">short</span> <span class="arg">id</span>)</p>

<h2>Animal Methods</h2>
<p class="method"><span class="type">bool</span> <span class="identifier">is_animal</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_pet</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">trained_level</span>()
    <span class="comment">**none=-1, semi-wild=0, trained=1, well trained=2, skillfully=3, expertly=4, exceptionally=5, masterfully=6, domesticated=7, unknown=8, wild=9, hostile=10</span></p>

<h2>Role Methods</h2>
<p class="method"><span class="type">float</span> <span class="identifier">get_role_rating</span>(<span class="type">string</span> <span class="arg">role_name</span>)</p>

<h2>Squad &amp; Military Methods</h2>
<p class="method"><span class="type">bool</span> <span class="identifier">can_assign_military</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">active_military</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">squad_id</span>() <span class="comment">**below 0 indicates no squad</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">squad_position</span>()</p>

<h2>Syndrome (Buff/Ailment) Methods</h2>
<p class="method"><span class="type">bool</span> <span class="identifier">is_buffed</span>()
    <span class="comment">**a buff is considered any syndrome that won't hospitalize the unit</span></p>
<p class="method"><span class="type">string</span> <span class="identifier">buffs</span>()
    <span class="comment">**returns a comma separated list of the current active beneficial syndromes</span></p>
<p class="method"><span class="type">string</span> <span class="identifier">syndromes</span>()
    <span class="comment">**returns a comma separated list of all active syndromes</span></p>
<p class="method"><span class="type">string</span> <span class="identifier">has_syndromes</span>(<span class="type">string</span> <span class="arg">name</span>)
    <span class="comment">**can be partial names</span></p>
)***";

static const char *attributes_doc = R"***(
<p class="method"><span class="type">int</span> <span class="identifier">attribute</span>(<span class="type">int</span> <span class="arg">attribute_id</span>)</p>
<p class="method"><span class="type">int</span> <span class="identifier">attribute_maximum</span>(<span class="type">int</span> <span class="arg">attribute_id</span>)</p>
<p class="method"><span class="type">int</span> <span class="identifier">strength</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">agility</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">toughness</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">endurance</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">recuperation</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">disease_resistance</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">analytical_ability</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">focus</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">willpower</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">creativity</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">intuition</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">patience</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">memory</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">linguistic_ability</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">spatial_sense</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">musicality</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">kinesthetic_sense</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">empathy</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">social_awareness</span>()</p>
)***";

static const char *health_doc = R"***(
<p class="method"><span class="type">bool</span> <span class="identifier">has_health_issue</span>(<span class="type">int</span> <span class="arg">health_category</span>, <span class="type">int</span> <span class="arg">descriptor_index</span>)
    <span class="comment">**use -1 as the descriptor_index to match anything in the category</span></p>
)***";

static const char *equipment_doc = R"***(
<p class="comment">**Use -1 to return totals for all items</p>
<p class="method"><span class="type">float</span> <span class="identifier">get_coverage_rating</span>(<span class="type">int</span> <span class="arg">item_type_id</span>)
    <span class="comment">**returns a percentage of basic clothing coverage (feet, legs, chest)</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_missing_equip_count</span>(<span class="type">int</span> <span class="arg">item_type_id</span>)
    <span class="comment">**returns the number of missing items</span></p>
<p class="method"><span class="type">float</span> <span class="identifier">get_uniform_rating</span>(<span class="type">int</span> <span class="arg">item_type_id</span>) 
    <span class="comment">**returns a percentage of the squad or work uniform equipped</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_inventory_wear</span>(<span class="type">int</span> <span class="arg">item_type_id</span>)
    <span class="comment">**returns a value from 0-3 for the most worn out item equipped, 3 being the worst</span></p>
)***";

static const char *personality_doc = R"***(
<p class="method"><span class="type">int</span> <span class="identifier">trait</span>(<span class="type">int</span> <span class="arg">trait_id</span>)</p>
<p class="method"><span class="type">int</span> <span class="identifier">belief_value</span>(<span class="type">int</span> <span class="arg">belief_id</span>)</p>
<p class="method"><span class="type">bool</span> <span class="identifier">has_goal</span>(<span class="type">int</span> <span class="arg">goal_id</span>)</p>
<p class="method"><span class="type">int</span> <span class="identifier">goals_realized</span>()
    <span class="comment">**returns the number of goals/dreams that have been realized</span></p>
)***";

static const char *profession_job_doc = R"***(
<p class="method"><span class="type">int</span> <span class="identifier">current_job_id</span>()</p>
<p class="method"><span class="type">string</span> <span class="identifier">profession</span>()</p>
<p class="method"><span class="type">int</span> <span class="identifier">raw_profession</span>()</p>
<p class="method"><span class="type">string</span> <span class="identifier">custom_profession_name</span>()</span></p>
)***";

static const char *labor_doc = R"***(
<p class="method"><span class="type">int</span> <span class="identifier">total_assigned_labors</span>(<span class="type">bool</span> <span class="arg">include_skill_less</span>)
    <span class="comment">**skill-less labors are primarily hauling, but also includes labors like lever pulling, recovering wounded, etc.</span></p>
<p class="method"><span class="type">bool</span> <span class="identifier">can_set_labors</span>()</p>
<p class="method"><span class="type">bool</span> <span class="identifier">labor_enabled</span>(<span class="type">int</span> <span class="arg">labor_id</span>)</p>
<p class="method"><span class="type">bool</span> <span class="identifier">is_labor_state_dirty</span>(<span class="type">int</span> <span class="arg">labor_id</span>)</p>
<p class="method"><span class="type">int</span> <span class="identifier">labor_rating</span>(<span class="type">int</span> <span class="arg">labor_id</span>)</p>
)***";

static const char *skill_doc = R"***(
<p class="method"><span class="type">float</span> <span class="identifier">skill_level</span>(<span class="type">int</span> <span class="arg">skill_id</span>)
    <span class="comment">**returns the skill level as seen in game (eg. 4)</span></p>
<p class="method"><span class="type">float</span> <span class="identifier">skill_level_precise</span>(<span class="type">int</span> <span class="arg">skill_id</span>)
    <span class="comment">**returns the interpolated skill level (eg. 4.7)</span></p>
<p class="method"><span class="type">float</span> <span class="identifier">skill_level_raw</span>(<span class="type">int</span> <span class="arg">skill_id</span>)
    <span class="comment">**returns the raw skill level, which can exceed  20 (eg. 120)</span></p>
<p class="method"><span class="type">float</span> <span class="identifier">skill_level_raw_precise</span>(<span class="type">int</span> <span class="arg">skill_id</span>)
    <span class="comment">**returns the interpolated raw skill level (eg. 120.9)</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">rust_level()</span>
    <span class="comment">**returns the worst level of rust found in the unit's skills, higher indicating more rust</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">total_skill_levels</span>()</p>
)***";

static const char *need_doc = R"***(
<p class="method"><span class="type">int</span> <span class="identifier">get_need_type_focus</span>(<span class="type">int</span> <span class="arg">need_id</span>)
    <span class="comment">**returns the current focus for the given need, or the lowest focus if they are multiple needs with the given id (e.g. prayer).</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_need_type_level</span>(<span class="type">int</span> <span class="arg">need_id</span>)
    <span class="comment">**returns the need level for the given need, or the sum of all levels if they are multiple needs with the given id (e.g. prayer).</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_need_focus</span>(<span class="type">int</span> <span class="arg">need_id</span>, <span class="type">int</span> <span class="arg">deity_id</span>)
    <span class="comment">**returns the current focus for the given need and deity (-1 for no deity).</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_need_level</span>(<span class="type">int</span> <span class="arg">need_id</span>, <span class="type">int</span> <span class="arg">deity_id</span>)
    <span class="comment">**returns the need level for the given need and deity (-1 for no deity).</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_need_focus_degree</span>(<span class="type">int</span> <span class="arg">need_id</span>, <span class="type">int</span> <span class="arg">deity_id</span>)
    <span class="comment">**returns the current focus degree for the given need and deity (-1 for no deity). 0 = badly distracted, 1 = distracted, 2 = unfocused, 3 = not distracted, 4 = untroubled, 5 = level-headed, 6 = unfettered</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_current_focus</span>()
    <span class="comment">**returns the current "overall" focus</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_undistracted_focus</span>()
    <span class="comment">**returns the undistracted (or "untroubled") focus</span></p>
<p class="method"><span class="type">int</span> <span class="identifier">get_focus_degree</span>()
    <span class="comment">**returns the current "overall" focus degree
        (0 = badly distracted, 1 = distracted, 2 = unfocused, 3 = untroubled, 4 = somewhat focused, 5 = quite focused, 6 = very focused)</span></p>
)***";

ScriptDialog::ScriptDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScriptDialog)
{
    ui->setupUi(this);

    AdaptiveColorFactory adaptive;

    // Set the static help text with a dynamic stylesheet so the colors are adapted to the current palette.

    QString stylesheet = QString(doc_style)
        .arg(adaptive.color(Qt::blue).name(QColor::HexRgb)) // code example color
        .arg(adaptive.color(Qt::red).name(QColor::HexRgb)) // type color
        .arg(adaptive.color(Qt::blue).name(QColor::HexRgb)); // arg color

    ui->text_help->document()->setDefaultStyleSheet(stylesheet);
    ui->text_help->document()->setHtml(general_doc);

    ui->txt_att_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_att_info->document()->setHtml(attributes_doc);

    ui->txt_health_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_health_info->document()->setHtml(health_doc);

    ui->txt_item_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_item_info->document()->setHtml(equipment_doc);

    ui->txt_personality_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_personality_info->document()->setHtml(personality_doc);

    ui->txt_job_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_job_info->document()->setHtml(profession_job_doc);

    ui->txt_labor_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_labor_info->document()->setHtml(labor_doc);

    ui->txt_skill_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_skill_info->document()->setHtml(skill_doc);

    ui->txt_need_info->document()->setDefaultStyleSheet(stylesheet);
    ui->txt_need_info->document()->setHtml(need_doc);

    //TODO: convert to tables/trees and add in a search (and sort?) function/options

    auto blue = adaptive.color(Qt::blue);
    auto row_html = QString("<tr><td><font color=%1>%2</font></td><td><b>%3</b></td></tr>").arg(blue.name(QColor::HexRgb));
    //LABORS
    GameDataReader *gdr = GameDataReader::ptr();
    QString labor_list = "<b>Labor Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Labor ID</th><th>Labor</th></tr>";
    foreach(Labor *l, gdr->get_ordered_labors()) {
        labor_list.append(row_html.arg(l->labor_id, 2, 10, QChar('0')).arg(l->name));
    }
    labor_list.append("</table>");
    ui->text_labors->append(labor_list);

    //SKILLS
    QString skill_list = "<b>Skills Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Skill ID</th><th>Skill</th></tr>";
    for (auto skill: gdr->get_ordered_skills(false)) {
        skill_list.append(row_html.arg(skill->id).arg(skill->name));
    }
    skill_list.append("</table>");
    ui->text_skills->append(skill_list);

    //ATTRIBUTES
    QString attribute_list = "<b>Attribute Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Attribute ID</th><th>Attribute</th></tr>";
    QPair<ATTRIBUTES_TYPE, QString> att_pair;
    foreach(att_pair, gdr->get_ordered_attribute_names()) {
        attribute_list.append(row_html.arg(att_pair.first).arg(att_pair.second));
    }
    attribute_list.append("</table>");
    ui->text_attributes->append(attribute_list);

    //PERSONALITY
    QString trait_list = "<b>Trait Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Trait ID</th><th>Trait</th></tr>";
    QPair<int, Trait*> trait_pair;
    foreach(trait_pair, gdr->get_ordered_traits()) {
        trait_list.append(row_html.arg(trait_pair.second->id()).arg(trait_pair.second->get_name()));
    }
    trait_list.append("</table>");
    ui->text_personality->append(trait_list);

    QString belief_list = "<b>Belief Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Belief ID</th><th>Belief</th></tr>";
    QPair<int, QString> belief_pair;
    foreach(belief_pair, gdr->get_ordered_beliefs()) {
        belief_list.append(row_html.arg(belief_pair.first).arg(belief_pair.second));
    }
    belief_list.append("</table>");
    ui->text_personality->append(belief_list);

    QString goal_list = "<b>Goal Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Goal ID</th><th>Goal</th></tr>";
    QPair<int, QString> goal_pair;
    foreach(goal_pair, gdr->get_ordered_goals()) {
        goal_list.append(row_html.arg(goal_pair.first).arg(goal_pair.second));
    }
    goal_list.append("</table>");
    ui->text_personality->append(goal_list);

    //JOBS/PROFESSIONS
    QString job_list = "<b>Job Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Job ID</th><th>Job</th></tr>";
    QPair<int, QString> job_pair;
    foreach(job_pair, gdr->get_ordered_jobs()) {
        job_list.append(row_html.arg(job_pair.first).arg(job_pair.second));
    }
    job_list.append("</table>");
    ui->text_jobs->append(job_list);

    //HEALTH
    QString health_list = "<b>Health Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Category ID</th><th>Title</th><th>Descriptors</th></tr>";

    QPair<eHealth::H_INFO,QString> cat_pair;
    foreach(cat_pair, UnitHealth::ordered_category_names()) {
        HealthCategory *hc = UnitHealth::get_display_categories().value(cat_pair.first);
        health_list.append(QString("<tr><td><font color=%1>%2</font></td><td><b>%3</b></td>")
            .arg(blue.name(QColor::HexRgb))
            .arg(hc->id())
            .arg(hc->name()));
        health_list.append("<td><table border=0 cellpadding=1 cellspacing=1 width=100%>");
        short idx = 0;
        foreach(HealthInfo *hi, hc->descriptions()){
            health_list.append(QString("<tr><td width=5%>%1</td><td>%2</td></tr>").arg(idx).arg(hi->description(false)));
            idx++;
        }
        health_list.append("</table></td></tr>");
    }
    health_list.append("</table>");
    ui->text_health->append(health_list);

    //EQUIPMENT/ITEMS
    QString item_list = "<b>Item Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Item Type ID</th><th>Name</th></tr>";
    QMap<QString,int> item_types;
    for(int i=0; i < NUM_OF_ITEM_TYPES; i++){
        item_types.insert(Item::get_item_name_plural(static_cast<ITEM_TYPE>(i)),i);
    }
    foreach(QString name, item_types.uniqueKeys()){
        item_list.append(row_html.arg(QString::number(item_types.value(name))).arg(name));
    }
    item_list.append("</table>");
    ui->text_items->append(item_list);

    //NEEDS
    QString need_list = "<b>Needs Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Need ID</th><th>Need</th></tr>";
    for (int i = 0; i < gdr->get_need_count(); ++i) {
        need_list.append(row_html.arg(i).arg(gdr->get_need_name(i)));
    }
    need_list.append("</table>");
    ui->text_needs->append(need_list);


    connect(ui->btn_apply, SIGNAL(clicked()), SLOT(apply_pressed()));
    connect(ui->btn_save, SIGNAL(clicked()), SLOT(save_pressed()));

    //reposition widgets
    reposition_horiz_splitters();
    reposition_cursors();
    //adjust script splitter
    ui->splitter_script->setStretchFactor(0,8);
    ui->splitter_script->setStretchFactor(1,1);
}

void ScriptDialog::reposition_horiz_splitters(){
    foreach(QSplitter *s, ui->tabInfo->findChildren<QSplitter*>()){
        if(s->orientation() != Qt::Horizontal)
            continue;
        s->setStretchFactor(0,1);
        s->setStretchFactor(1,3);
    }
}

void ScriptDialog::reposition_cursors(){
    foreach(QTextEdit *te, ui->tabInfo->findChildren<QTextEdit*>()){
        te->moveCursor(QTextCursor::Start);
        te->ensureCursorVisible();
    }
}

ScriptDialog::~ScriptDialog(){
    delete ui;
}

void ScriptDialog::clear_script() {
    ui->script_edit->clear();
    ui->txt_script_name->clear();
    ui->lbl_save_status->clear();
    m_name = "";
}

void ScriptDialog::load_script(QString name, QString script){
    ui->script_edit->setPlainText(script);
    m_name = name;
    ui->txt_script_name->setText(m_name);
    ui->lbl_save_status->clear();
}

void ScriptDialog::apply_pressed() {
    ui->lbl_save_status->clear();
    if(script_is_valid()){
        emit test_script(ui->script_edit->toPlainText());
        ui->lbl_save_status->setText(tr("Script has been applied but hasn't been saved."));
    }
}

bool ScriptDialog::script_is_valid(){
    QString script = ui->script_edit->toPlainText().trimmed();
    Dwarf *m_dwarf = 0;
    if(DT->get_dwarves().count()>0){
        m_dwarf = DT->get_dwarves().at(0);
    }
    AdaptiveColorFactory color;
    QString error = QString("<font color=\"%1\">%2</font>").arg(color.color(Qt::red).name());
    QString warning = QString("<font color=\"%1\">%2</font>").arg(color.color(QColor::fromHsv(30, 255, 255)).name());
    QString success = QString("<font color=\"%1\">%2</font>").arg(color.color(Qt::green).name());

    if(!script.isEmpty() && m_dwarf){
        QJSEngine m_engine;
        QJSValue d_obj = m_engine.newQObject(m_dwarf);
        m_engine.globalObject().setProperty("d", d_obj);
        QJSValue ret = m_engine.evaluate(script);
        if(!ret.isBool()){
            QString err_msg;
            if(ret.isError()) {
                err_msg = tr("%1: %2<br/>%3")
                                 .arg(ret.property("name").toString())
                                 .arg(ret.property("message").toString())
                                 .arg(ret.property("stack").toString().replace("\n", "<br/>"));
            }else{
                m_engine.globalObject().setProperty("__internal_script_return_value_check", ret);
                err_msg = tr("Script returned %1 instead of boolean")
                                 .arg(m_engine.evaluate(QString("typeof __internal_script_return_value_check")).toString());
                m_engine.globalObject().deleteProperty("__internal_script_return_value_check");
            }
            ui->txt_script_log->setText(error.arg(err_msg));
            return false;
        }
        else {
            ui->txt_script_log->setText(success.arg(tr("The script is valid.")));
        }
    }
    else if (!m_dwarf) {
        ui->txt_script_log->setText(warning.arg(tr("Cannot test script without a dwarf.")));
    }
    else {
        ui->txt_script_log->setText(warning.arg(tr("The script is empty.")));
    }
    return true;
}

void ScriptDialog::save_pressed() {
    QString m_old_name = m_name;
    m_name = ui->txt_script_name->text();

    QSettings *s = DT->user_settings();
    int answer = QMessageBox::Yes;

    if(m_old_name != m_name){
        s->beginGroup("filter_scripts");
        foreach(QString script_name, s->childKeys()){
            if(m_name==script_name){
                answer = QMessageBox::question(0,"Confirm Replace",
                                               tr("A script with this name already exists and will be overwritten. Continue?"),
                                               QMessageBox::Yes,QMessageBox::No);
                break;
            }
        }
        s->endGroup();
    }

    if(answer == QMessageBox::No){
        ui->lbl_save_status->setText(tr("Save cancelled."));
        return;
    }

    if(m_old_name != m_name && m_old_name != "")
        s->remove(QString("filter_scripts/%1").arg(m_old_name));

    s->setValue(QString("filter_scripts/%1").arg(m_name), ui->script_edit->toPlainText());
    emit scripts_changed();

    ui->lbl_save_status->setText(tr("Script saved successfully!"));
}

void ScriptDialog::close_pressed(){
    this->reject();
}
