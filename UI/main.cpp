#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/spinctrl.h>
#include <wx/tokenzr.h>
#include <wx/dcbuffer.h>

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <ctime>
#include <memory>
#include <random>
#include <string>

#include "../structures/ArraySequence.hpp"
#include "AVL_tree.hpp"
#include "histogram.hpp"
#include "person.hpp"
#include "person_index.hpp"


static wxString to_utf8(const char* s){ return wxString::FromUTF8(s); }
static wxString from_std(const std::string& s){ return wxString::FromUTF8(s.c_str()); }
static std::string to_std(const wxString& s){ return std::string(s.utf8_str()); }

static wxString num_to_str(double v) { return wxString::Format("%g", v); }

static void show_error(const wxString& text) {
    wxMessageBox(text, to_utf8("Ошибка"), wxICON_WARNING);
}

static bool parse_one(wxString text, double& result) {
    text.Trim(true).Trim(false);
    text.Replace(",", ".");
    return text.ToCDouble(&result);
}

static bool parse_many(const wxString& text, ArraySequence<double>& result, wxString& bad) {
    wxStringTokenizer tokens(text, " \t\r\n;");
    while (tokens.HasMoreTokens()) {
        wxString word = tokens.GetNextToken();
        double value = 0;
        if (!word.ToCDouble(&value)) {
            bad = word;
            return false;
        }
        result.Append(value);
    }
    return true;
}

static void add_field(wxFlexGridSizer* grid, wxWindow* parent, const char* label, wxWindow* field) {
    grid->Add(new wxStaticText(parent, wxID_ANY, to_utf8(label)), 0, wxALIGN_CENTER_VERTICAL);
    grid->Add(field, 1, wxEXPAND);
}


class chart_panel : public wxPanel {
public:
    struct bar {
        wxString label;
        std::size_t count = 0;
    };

    chart_panel(wxWindow* parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 260)) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetMinSize(wxSize(300, 220));
        Bind(wxEVT_PAINT, &chart_panel::on_paint, this);
        Bind(wxEVT_SIZE, &chart_panel::on_resize, this);
    }

    void set_bars(const ArraySequence<bar>& bars) {
        m_bars = bars;
        Refresh();
    }

private:
    ArraySequence<bar> m_bars;

    void on_resize(wxSizeEvent& event) {
        Refresh();
        event.Skip();
    }

    void on_paint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();

        const int left = 50, right = 15, top = 20, bottom = 40;
        const wxSize size = GetClientSize();
        const int width = size.x - left - right;
        const int height = size.y - top - bottom;
        const std::size_t n = m_bars.GetLength();

        if (n == 0 || width <= 10 || height <= 10) {
            dc.DrawText(to_utf8("Нет данных"), 10, 10);
            return;
        }

        std::size_t max_count = 1;
        for (std::size_t i = 0; i < n; ++i) {
            max_count = std::max(max_count, m_bars.GetRef(i).count);
        }

        dc.SetPen(*wxBLACK_PEN);
        dc.DrawLine(left, top, left, top + height);
        dc.DrawLine(left, top + height, left + width, top + height);
        dc.DrawText(wxString::Format("%lu", (unsigned long)max_count), 5, top - 8);
        dc.DrawText("0", left - 14, top + height - 8);

        const double bar_width = double(width) / double(n);
        const int label_every = std::max(1, (int)std::ceil(60.0 / bar_width));

        dc.SetBrush(wxBrush(wxColour(70, 130, 180)));
        for (std::size_t i = 0; i < n; ++i) {
            const bar& b = m_bars.GetRef(i);
            const int x0 = left + int(i * bar_width);
            const int x1 = left + int((i + 1) * bar_width);
            const int h = int(height * double(b.count) / double(max_count));

            dc.SetPen(*wxWHITE_PEN);
            dc.DrawRectangle(x0, top + height - h, std::max(1, x1 - x0), h);

            dc.SetPen(*wxBLACK_PEN);
            if (bar_width >= 24 && b.count > 0) {
                wxString count_text = wxString::Format("%lu", (unsigned long)b.count);
                wxSize text_size = dc.GetTextExtent(count_text);
                dc.DrawText(count_text, x0 + (x1 - x0 - text_size.x) / 2,
                            top + height - h - text_size.y - 1);
            }
            if (i % label_every == 0) {
                dc.DrawText(b.label, x0, top + height + 4);
            }
        }
    }
};


class people_panel : public wxPanel {
public:
    people_panel(wxWindow* parent) : wxPanel(parent) {
        auto* add_box = new wxStaticBoxSizer(wxVERTICAL, this, to_utf8("Добавить человека"));
        wxWindow* add_parent = add_box->GetStaticBox();

        m_series = new wxTextCtrl(add_parent, wxID_ANY);
        m_number = new wxTextCtrl(add_parent, wxID_ANY);
        m_last = new wxTextCtrl(add_parent, wxID_ANY);
        m_first = new wxTextCtrl(add_parent, wxID_ANY);
        m_patr = new wxTextCtrl(add_parent, wxID_ANY);
        m_date = new wxDatePickerCtrl(add_parent, wxID_ANY, wxDateTime(1, wxDateTime::Jan, 2000));

        auto* add_grid = new wxFlexGridSizer(2, 5, 5);
        add_grid->AddGrowableCol(1);
        add_field(add_grid, add_parent, "Серия паспорта", m_series);
        add_field(add_grid, add_parent, "Номер паспорта", m_number);
        add_field(add_grid, add_parent, "Фамилия", m_last);
        add_field(add_grid, add_parent, "Имя", m_first);
        add_field(add_grid, add_parent, "Отчество", m_patr);
        add_field(add_grid, add_parent, "Дата рождения", m_date);

        auto* add_button = new wxButton(add_parent, wxID_ANY, to_utf8("Добавить"));
        add_box->Add(add_grid, 0, wxEXPAND | wxALL, 5);
        add_box->Add(add_button, 0, wxALIGN_RIGHT | wxALL, 5);

        auto* find_box = new wxStaticBoxSizer(wxVERTICAL, this, to_utf8("Поиск (по индексу AVL)"));
        wxWindow* find_parent = find_box->GetStaticBox();

        wxArrayString fields;
        fields.Add(to_utf8("Имя"));
        fields.Add(to_utf8("Фамилия"));
        fields.Add(to_utf8("ФИО (инициалы: «И И И»)"));
        fields.Add(to_utf8("Возраст"));
        m_field = new wxChoice(find_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, fields);
        m_field->SetSelection(0);
        m_query = new wxTextCtrl(find_parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                 wxTE_PROCESS_ENTER);
        auto* find_button = new wxButton(find_parent, wxID_ANY, to_utf8("Найти"));

        find_box->Add(new wxStaticText(find_parent, wxID_ANY, to_utf8("Искать по:")), 0, wxALL, 5);
        find_box->Add(m_field, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
        find_box->Add(new wxStaticText(find_parent, wxID_ANY, to_utf8("Значение:")), 0, wxALL, 5);
        find_box->Add(m_query, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
        find_box->Add(find_button, 0, wxALIGN_RIGHT | wxALL, 5);

        auto* top = new wxBoxSizer(wxHORIZONTAL);
        top->Add(add_box, 1, wxEXPAND | wxALL, 5);
        top->Add(find_box, 1, wxEXPAND | wxALL, 5);

        m_all = make_table();
        m_found = make_table();
        m_status = new wxStaticText(this, wxID_ANY, "");

        auto* root = new wxBoxSizer(wxVERTICAL);
        root->Add(top, 0, wxEXPAND);
        root->Add(new wxStaticText(this, wxID_ANY, to_utf8("Все люди:")), 0, wxLEFT | wxTOP, 8);
        root->Add(m_all, 1, wxEXPAND | wxALL, 5);
        root->Add(new wxStaticText(this, wxID_ANY, to_utf8("Результаты поиска:")), 0, wxLEFT, 8);
        root->Add(m_found, 1, wxEXPAND | wxALL, 5);
        root->Add(m_status, 0, wxEXPAND | wxALL, 5);
        SetSizer(root);

        add_button->Bind(wxEVT_BUTTON, &people_panel::on_add, this);
        find_button->Bind(wxEVT_BUTTON, &people_panel::on_find, this);
        m_query->Bind(wxEVT_TEXT_ENTER, &people_panel::on_find, this);
    }

private:
    ArraySequence<person> m_persons;
    person_index m_index;

    wxTextCtrl *m_series, *m_number, *m_last, *m_first, *m_patr, *m_query;
    wxDatePickerCtrl* m_date;
    wxChoice* m_field;
    wxListCtrl *m_all, *m_found;
    wxStaticText* m_status;

    wxListCtrl* make_table() {
        auto* table = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 130),
                                     wxLC_REPORT | wxLC_SINGLE_SEL);
        table->InsertColumn(0, "#", wxLIST_FORMAT_LEFT, 40);
        table->InsertColumn(1, to_utf8("ФИО"), wxLIST_FORMAT_LEFT, 280);
        table->InsertColumn(2, to_utf8("Паспорт"), wxLIST_FORMAT_LEFT, 130);
        table->InsertColumn(3, to_utf8("Дата рождения"), wxLIST_FORMAT_LEFT, 120);
        table->InsertColumn(4, to_utf8("Возраст"), wxLIST_FORMAT_LEFT, 80);
        return table;
    }

    void add_row(wxListCtrl* table, std::size_t number, const person& p) {
        long row = table->InsertItem(table->GetItemCount(),
                                     wxString::Format("%lu", (unsigned long)(number + 1)));
        person_id id = p.get_passport();

        table->SetItem(row, 1, from_std(p.get_full_name()));
        table->SetItem(row, 2, wxString::Format("%lu %lu", (unsigned long)id.series,
                                                (unsigned long)id.number));
        table->SetItem(row, 3, wxDateTime(static_cast<time_t>(p.get_date())).FormatISODate());
        table->SetItem(row, 4, wxString::Format("%lu", (unsigned long)p.get_age()));
    }

    void rebuild_index() {
        m_index = person_index();
        m_index.build(m_persons);
    }

    void show_all_persons() {
        m_all->DeleteAllItems();
        for (std::size_t i = 0; i < m_persons.GetLength(); ++i) {
            add_row(m_all, i, m_persons.Get(i));
        }
    }

    void on_add(wxCommandEvent&) {
        unsigned long series = 0, number = 0;
        if (!m_series->GetValue().Trim().ToULong(&series) ||
            !m_number->GetValue().Trim().ToULong(&number)) {
            show_error(to_utf8("Серия и номер паспорта должны быть целыми числами."));
            return;
        }

        wxString last = m_last->GetValue().Trim().Trim(false);
        wxString first = m_first->GetValue().Trim().Trim(false);
        wxString patr = m_patr->GetValue().Trim().Trim(false);
        if (last.empty() || first.empty() || patr.empty()) {
            show_error(to_utf8("Заполните фамилию, имя и отчество."));
            return;
        }

        for (std::size_t i = 0; i < m_persons.GetLength(); ++i) {
            person_id id = m_persons.Get(i).get_passport();
            if (id.series == series && id.number == number) {
                show_error(to_utf8("Человек с таким паспортом уже есть."));
                return;
            }
        }

        try {
            person_id id{static_cast<std::size_t>(series), static_cast<std::size_t>(number)};
            person p(id, to_std(first), to_std(last), to_std(patr),
                     static_cast<std::time_t>(m_date->GetValue().GetTicks()));

            m_persons.Append(p);
            rebuild_index();
            show_all_persons();

            m_status->SetLabel(to_utf8("Добавлено. Всего людей: ") +
                               wxString::Format("%lu", (unsigned long)m_persons.GetLength()));
        } catch (const exception& e) {
            show_error(from_std(e.what()));
        } catch (const std::exception& e) {
            show_error(from_std(e.what()));
        }
    }

    void on_find(wxCommandEvent&) {
        m_found->DeleteAllItems();
        const std::string query = to_std(m_query->GetValue().Trim().Trim(false));

        try {
            ArraySequence<const person*> result;

            switch (m_field->GetSelection()) {
                case 0: result = m_index.find_by_first_name(query); break;
                case 1: result = m_index.find_by_last_name(query); break;
                case 2: result = m_index.find_by_FIO(query); break;
                case 3: {
                    unsigned long age = 0;
                    if (!m_query->GetValue().Trim().ToULong(&age)) {
                        show_error(to_utf8("Возраст должен быть целым числом."));
                        return;
                    }
                    result = m_index.find_by_age(static_cast<std::size_t>(age));
                    break;
                }
                default: return;
            }

            for (std::size_t i = 0; i < result.GetLength(); ++i) {
                const person* p = result.Get(i);
                add_row(m_found, i, *p);
            }
            m_status->SetLabel(to_utf8("Найдено: ") +
                               wxString::Format("%lu", (unsigned long)result.GetLength()));
        } catch (const exception& e) {
            show_error(from_std(e.what()));
        } catch (const std::exception& e) {
            show_error(from_std(e.what()));
        }
    }
};


class histogram_panel : public wxPanel {
public:
    histogram_panel(wxWindow* parent) : wxPanel(parent), m_rng(std::random_device{}()) {
        auto* param_box = new wxStaticBoxSizer(wxVERTICAL, this, to_utf8("Параметры гистограммы"));
        wxWindow* param_parent = param_box->GetStaticBox();

        wxArrayString modes;
        modes.Add(to_utf8("Равные интервалы"));
        modes.Add(to_utf8("Свои границы"));
        m_mode = new wxRadioBox(param_parent, wxID_ANY, to_utf8("Режим"), wxDefaultPosition,
                                wxDefaultSize, modes, 1, wxRA_SPECIFY_COLS);

        m_min = new wxTextCtrl(param_parent, wxID_ANY, "0");
        m_max = new wxTextCtrl(param_parent, wxID_ANY, "100");
        m_step = new wxTextCtrl(param_parent, wxID_ANY, "10");
        m_bounds = new wxTextCtrl(param_parent, wxID_ANY, "0 10 25 50 100");

        auto* grid = new wxFlexGridSizer(2, 5, 5);
        grid->AddGrowableCol(1);
        add_field(grid, param_parent, "Мин", m_min);
        add_field(grid, param_parent, "Макс", m_max);
        add_field(grid, param_parent, "Шаг", m_step);
        add_field(grid, param_parent, "Границы", m_bounds);

        auto* create_button = new wxButton(param_parent, wxID_ANY, to_utf8("Создать / сбросить"));

        param_box->Add(m_mode, 0, wxEXPAND | wxALL, 5);
        param_box->Add(grid, 0, wxEXPAND | wxALL, 5);
        param_box->Add(create_button, 0, wxALIGN_RIGHT | wxALL, 5);

        auto* data_box = new wxStaticBoxSizer(wxVERTICAL, this, to_utf8("Данные"));
        wxWindow* data_parent = data_box->GetStaticBox();

        m_values = new wxTextCtrl(data_parent, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 80),
                                  wxTE_MULTILINE);
        auto* add_button = new wxButton(data_parent, wxID_ANY, to_utf8("Добавить значения"));
        m_count = new wxSpinCtrl(data_parent, wxID_ANY, "200", wxDefaultPosition, wxDefaultSize,
                                 wxSP_ARROW_KEYS, 1, 100000, 200);
        auto* random_button = new wxButton(data_parent, wxID_ANY, to_utf8("Случайные N"));

        auto* random_row = new wxBoxSizer(wxHORIZONTAL);
        random_row->Add(m_count, 1, wxRIGHT, 5);
        random_row->Add(random_button, 0);

        data_box->Add(new wxStaticText(data_parent, wxID_ANY,
                                       to_utf8("Числа через пробел или «;»\n(десятичная точка):")),
                      0, wxALL, 5);
        data_box->Add(m_values, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
        data_box->Add(add_button, 0, wxALIGN_RIGHT | wxALL, 5);
        data_box->Add(random_row, 0, wxEXPAND | wxALL, 5);

        auto* left = new wxBoxSizer(wxVERTICAL);
        left->Add(param_box, 0, wxEXPAND | wxALL, 5);
        left->Add(data_box, 0, wxEXPAND | wxALL, 5);

        m_table = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 200),
                                 wxLC_REPORT | wxLC_SINGLE_SEL);
        m_table->InsertColumn(0, to_utf8("Интервал"), wxLIST_FORMAT_LEFT, 140);
        m_table->InsertColumn(1, to_utf8("Кол-во"), wxLIST_FORMAT_LEFT, 60);
        m_table->InsertColumn(2, to_utf8("Среднее"), wxLIST_FORMAT_LEFT, 80);
        m_table->InsertColumn(3, to_utf8("Дисперсия"), wxLIST_FORMAT_LEFT, 90);
        m_table->InsertColumn(4, to_utf8("Медиана"), wxLIST_FORMAT_LEFT, 80);
        m_table->InsertColumn(5, to_utf8("Мин"), wxLIST_FORMAT_LEFT, 70);
        m_table->InsertColumn(6, to_utf8("Макс"), wxLIST_FORMAT_LEFT, 70);

        m_chart = new chart_panel(this);
        m_status = new wxStaticText(this, wxID_ANY, to_utf8("Создайте гистограмму."));

        auto* right = new wxBoxSizer(wxVERTICAL);
        right->Add(m_table, 1, wxEXPAND | wxALL, 5);
        right->Add(m_chart, 1, wxEXPAND | wxALL, 5);
        right->Add(m_status, 0, wxEXPAND | wxALL, 5);

        auto* root = new wxBoxSizer(wxHORIZONTAL);
        root->Add(left, 0, wxEXPAND);
        root->Add(right, 1, wxEXPAND);
        SetSizer(root);

        update_fields();
        m_mode->Bind(wxEVT_RADIOBOX, &histogram_panel::on_mode_changed, this);
        create_button->Bind(wxEVT_BUTTON, &histogram_panel::on_create, this);
        add_button->Bind(wxEVT_BUTTON, &histogram_panel::on_add_values, this);
        random_button->Bind(wxEVT_BUTTON, &histogram_panel::on_random, this);
    }

private:
    static constexpr double k_max_bins = 1000;

    wxRadioBox* m_mode;
    wxTextCtrl *m_min, *m_max, *m_step, *m_bounds, *m_values;
    wxSpinCtrl* m_count;
    wxListCtrl* m_table;
    chart_panel* m_chart;
    wxStaticText* m_status;

    std::unique_ptr<histogram<double>> m_hist;
    std::mt19937 m_rng;

    void update_fields() {
        const bool equal = (m_mode->GetSelection() == 0);
        m_min->Enable(equal);
        m_max->Enable(equal);
        m_step->Enable(equal);
        m_bounds->Enable(!equal);
    }

    void on_mode_changed(wxCommandEvent&) {
        update_fields();
    }

    void on_create(wxCommandEvent&) {
        try {
            if (m_mode->GetSelection() == 0) {
                double lo, hi, step;
                if (!parse_one(m_min->GetValue(), lo) ||
                    !parse_one(m_max->GetValue(), hi) ||
                    !parse_one(m_step->GetValue(), step)) {
                    show_error(to_utf8("Мин, макс и шаг должны быть числами."));
                    return;
                }
                if (!(lo < hi) || !(step > 0)) {
                    show_error(to_utf8("Нужно: мин < макс и шаг > 0."));
                    return;
                }
                if (std::ceil((hi - lo) / step) > k_max_bins) {
                    show_error(to_utf8("Слишком много интервалов (максимум 1000)."));
                    return;
                }
                m_hist = std::make_unique<histogram<double>>(lo, hi, step);
            } else {
                ArraySequence<double> bounds;
                wxString bad;
                if (!parse_many(m_bounds->GetValue(), bounds, bad)) {
                    show_error(to_utf8("Не удалось разобрать число: ") + bad);
                    return;
                }
                if (bounds.GetLength() > k_max_bins) {
                    show_error(to_utf8("Слишком много границ (максимум 1000)."));
                    return;
                }
                m_hist = std::make_unique<histogram<double>>(bounds);
            }

            m_status->SetLabel(to_utf8("Гистограмма создана."));
            show_histogram();
        } catch (const exception& e) {
            show_error(from_std(e.what()));
        } catch (const std::exception& e) {
            show_error(from_std(e.what()));
        }
    }

    void report(std::size_t added, std::size_t skipped) {
        show_histogram();
        wxString text = to_utf8("Добавлено: ") + wxString::Format("%lu", (unsigned long)added);
        if (skipped > 0) {
            text += to_utf8(", вне диапазона (пропущено): ") +
                    wxString::Format("%lu", (unsigned long)skipped);
        }
        m_status->SetLabel(text);
    }

    void on_add_values(wxCommandEvent&) {
        if (!m_hist) {
            show_error(to_utf8("Сначала создайте гистограмму."));
            return;
        }

        ArraySequence<double> values;
        wxString bad;
        if (!parse_many(m_values->GetValue(), values, bad)) {
            show_error(to_utf8("Не удалось разобрать число: ") + bad);
            return;
        }

        try {
            std::size_t added = 0;
            std::size_t skipped = 0;

            for (std::size_t i = 0; i < values.GetLength(); ++i) {
                if (m_hist->add(values.Get(i))) {
                    ++added;
                } else {
                    ++skipped;
                }
            }

            report(added, skipped);
            m_values->Clear();
        } catch (const exception& e) {
            show_error(from_std(e.what()));
        } catch (const std::exception& e) {
            show_error(from_std(e.what()));
        }
    }

    void on_random(wxCommandEvent&) {
        if (!m_hist) {
            show_error(to_utf8("Сначала создайте гистограмму."));
            return;
        }

        const double lo = m_hist->get_min();
        const double hi = m_hist->get_max();
        std::normal_distribution<double> distribution((lo + hi) / 2.0, (hi - lo) / 6.0);

        try {
            std::size_t added = 0;
            for (int i = 0; i < m_count->GetValue(); ++i) {
                for (int attempt = 0; attempt < 100; ++attempt) {
                    if (m_hist->add(distribution(m_rng))) {
                        ++added;
                        break;
                    }
                }
            }
            report(added, 0);
        } catch (const exception& e) {
            show_error(from_std(e.what()));
        } catch (const std::exception& e) {
            show_error(from_std(e.what()));
        }
    }

    void show_histogram() {
        m_table->DeleteAllItems();
        ArraySequence<chart_panel::bar> bars;

        if (!m_hist) {
            m_chart->set_bars(bars);
            return;
        }

        const selection_histogram<double>& sections = m_hist->get_selection();
        const std::size_t n = sections.get_bin_count();

        for (std::size_t i = 0; i < n; ++i) {
            const range_statistic<double> stat = m_hist->get_bin(i);

            const wxString lower = num_to_str(sections.get_lower(i));
            const wxString upper = num_to_str(sections.get_upper(i));
            const wxString label = "[" + lower + "; " + upper + (i + 1 == n ? "]" : ")");

            long row = m_table->InsertItem(m_table->GetItemCount(), label);
            m_table->SetItem(row, 1, wxString::Format("%lu", (unsigned long)stat.m_count));
            if (stat.m_count > 0) {
                m_table->SetItem(row, 2, wxString::Format("%.4g", stat.mean()));
                m_table->SetItem(row, 3, wxString::Format("%.4g", stat.variance()));
                m_table->SetItem(row, 4, wxString::Format("%.4g", stat.median()));
                m_table->SetItem(row, 5, num_to_str(stat.m_min_elem));
                m_table->SetItem(row, 6, num_to_str(stat.m_max_elem));
            } else {
                for (int column = 2; column <= 6; ++column) {
                    m_table->SetItem(row, column, "-");
                }
            }

            chart_panel::bar b;
            b.label = lower;
            b.count = stat.m_count;
            bars.Append(b);
        }

        m_chart->set_bars(bars);
    }
};


class main_frame : public wxFrame {
public:
    main_frame() : wxFrame(nullptr, wxID_ANY, to_utf8("AVL: индекс людей и гистограмма"),
                           wxDefaultPosition, wxSize(1150, 760)) {
        auto* tabs = new wxNotebook(this, wxID_ANY);
        tabs->AddPage(new people_panel(tabs), to_utf8("Люди"), true);
        tabs->AddPage(new histogram_panel(tabs), to_utf8("Гистограмма"));
        CreateStatusBar();
        SetStatusText(to_utf8("Готово"));
        Centre();
    }
};

class app : public wxApp {
public:
    bool OnInit() override {
        if (!wxApp::OnInit()) return false;
        (new main_frame())->Show();
        return true;
    }
};

wxIMPLEMENT_APP(app);
