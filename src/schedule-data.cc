/*
 * main.cc
 * Copyright (C) 2021 Azael Reyes <azael.devel@gmail.com>
 * 
 * octetos-schedule is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * octetos-schedule is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sstream>
#include <fstream>
#include <iostream>
#include <locale>
#include <ctime>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>


#include "schedule.hh"

#if defined(_WIN32) || defined(_WIN64)

extern "C" char* strptime(const char* s,const char* f,struct tm* tm) 
{
	std::istringstream input(s);
	input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
	input >> std::get_time(tm, f);
	if (input.fail()) {
		return nullptr;
	}
	return (char*)(s + input.tellg());
}

#endif

namespace oct::core
{
	Person::Person(const std::string& n,const std::string& ap,const std::string& am)
	{
		name = n + " " + ap + " " + am;
	}
	Person::Person(const std::string& n)
	{
		name = n;
	}
	Person::Person()
	{
	}
	 
	const std::string& Person::operator =(const std::string& n)
	{
		name = n;
		
		return n;
	}
	const std::string& Person::get_name()const
	{
		return name;
	}

	DataTime::DataTime()
	{
		(tm&)*this = {0};
	}
	DataTime::DataTime(const tm& t)
	{
		(tm&)*this = t;
	}
	DataTime::DataTime(const DataTime& dt)
	{
		(tm&)*this = (tm&)dt;
	}
	
	const time_t* DataTime::operator =(const time_t* t)
	{
		tm* thistm = localtime(t);
		(tm&)*this = *thistm;
		return t;
	}
	const tm& DataTime::operator =(const tm& t)
	{
		(tm&)*this = t;
		return t;
	}
	bool DataTime::operator ==(const DataTime& o)const
	{
		if(tm_sec != o.tm_sec) return false;
		else if(tm_min != o.tm_min) return false;
		else if(tm_hour != o.tm_hour) return false;
		else if(tm_mday != o.tm_mday) return false;
		else if(tm_mon != o.tm_mon) return false;
		else if(tm_year != o.tm_year) return false;
		
		return true;
	}
	
	int DataTime::get_week_day()const
	{
		return tm_wday;
	}
	double DataTime::diff(const DataTime& dt)const
	{
		time_t tm1,tm2;
		tm t1 = *this;
		tm t2 = dt;
		tm1 = mktime(&t1);
		tm2 = mktime(&t2);
		return difftime(tm2, tm1);		
	}
	
	void DataTime::print(std::ostream& out, const std::string& format) const
	{
		out << std::put_time(this, format.c_str());
	}
	void DataTime::addSeconds(time_t s)
	{
		time_t t = mktime(this);
		t += s;
		tm* newt = localtime(&t);
		operator =(*newt);
	}
	
	void DataTime::read(const std::string& time, const std::string& format)
	{
		strptime(time.c_str(),format.c_str(),this);
	}
}

namespace oct::ec::sche
{

	Day::Day()
	{
	}
	Day::Day(const Day& d)
	{
		//std::cout << "Day::Day(const Day& d) - Step 1.0\n";
		if(size() > 24) core::Exception("El dia tiene un maximo de 24 horas",__FILE__,__LINE__);
		
		for(const core::DataTime& dt : d)
		{
			push_back(dt);
		}
		//std::cout << "Day::Day(const Day& d) - Step 2.0\n";
		//blocks.clear();
		sort();//ordena y genera bloques
		//std::cout << "Day::Day(const Day& d) - Step 3.0\n";
	}
	Day& Day::operator =(const Day& d)
	{
		//std::cout << "Day::operator = - Step 1.0\n";
		if(size() > 24) core::Exception("El dia tiene un maximo de 24 horas",__FILE__,__LINE__);
		
		for(const core::DataTime& dt : d)
		{
			push_back(dt);
		}
		//std::cout << "Day::operator = - Step 2.0\n";
		//blocks.clear();
		sort();//ordena y genera bloques
		//std::cout << "Day::operator = - Step 3.0\n";
		return *this;
	}
	Day::Blocks& Day::get_blocks()
	{
		return blocks;
	}
	const Day::Blocks& Day::get_blocks() const
	{
		return blocks;
	}
	Day& Day::inters(const Day& comp1, const Day& comp2)
	{
		if(comp1.size() > 24) core::Exception("El dia tiene un maximo de 24 horas",__FILE__,__LINE__);
		if(comp2.size() > 24) core::Exception("El dia tiene un maximo de 24 horas",__FILE__,__LINE__);
		
		for(const core::DataTime& tdt : comp1)
		{
			for(const core::DataTime& cdt : comp2)
			{
				//if(tdt.tm_wday != cdt.tm_wday) throw core::Exception("Los objetos indicado, tiene dias distinto",__FILE__,__LINE__);
				
				if(tdt.tm_hour == cdt.tm_hour) 
				{
					push_back(cdt);
				}
			}
		}
		//blocks.clear();
		sort();
		return *this;		
	}
	bool Day::haveDisponible()const
	{
		return std::list<core::DataTime>::size() > 0 ? true : false;
	}
	
	std::list<core::DataTime>::iterator Day::blocking(std::list<core::DataTime>::iterator b)
	{
		if(size() == 1)
		{
			Block block;
			block.push_back(&front());
			blocks.push_back(block);
			return end();
		}

		
		iterator itPrev = b;
		iterator itActual = b;
		itActual++;
		Block block;
		block.push_back(&*itPrev);
		while(itActual != end())
		{			
			if((*itActual).tm_hour == (*itPrev).tm_hour + 1)
			{
				block.push_back(&*itActual);
			}
			else 
			{
				blocks.push_back(block);
				return itActual;
			}
			
			itActual++;
			itPrev++;			
		}
		blocks.push_back(block);
		return itActual;
	}
	bool cmpHour(const core::DataTime& firts,const core::DataTime& second)
	{
		return firts.tm_hour < second.tm_hour;
	}
	void Day::sort()
	{
		//ordenar los elementos core::DataTime por su valor tm_wday
		std::list<core::DataTime>::sort(cmpHour);
		
		//contruir bloques de horas continuas
		if(size() == 0) return;//no hay nada que ordenar si hay 1 o 0 elementos
		
		iterator it = begin();
		while(it != end())
		{
			it = blocking(it);
		}
	}
	void Day::add(const Block& b)
	{
		Block newb;
		for(const core::DataTime* dt : b)
		{
			push_back(*dt);
			newb.push_back(&back());	
		}
		blocks.push_back(newb);
	}	
	void Day::combns(std::list<Day>& days, unsigned int hours)const
	{
		if(hours < 1) throw core::Exception("No esta permitido bloques de 0",__FILE__,__LINE__);		
		if(size() > 24) core::Exception("El dia tiene un maximo de 24 horas",__FILE__,__LINE__);
		
		//std::cout << "Day::combns - Step 1\n";
		unsigned int countHB = 0;
		for(const Block& b : blocks)
		{
			countHB += b.size();
		}
		if(countHB != size()) 
		{
			std::string msg = "Los bloques contiene un total de '";
			msg += std::to_string(countHB) + ", sin embargo, el dia contiene " + std::to_string(size()) + " horas.";
			throw core::Exception(msg,__FILE__,__LINE__);
		}
		//std::cout << "Day::combns - Step 2.0\n";
		Day d;
		//std::cout << "Day::combns - Step 2.1\n";
		days.push_back(d);
		//std::cout << "Day::combns - Step 2.2\n";
		Day& day = days.back();//bloques de combinacion generados
		//std::cout << "Day::combns - Step 3\n";
		for(const Block& block : blocks)
		{
			//bloques equivalentes al numero de horas pedidos
			if(block.size() == hours)
			{
				day.add(block);
				continue;//estas horas ya fueron repartidas
			}
			
			//bloques con multiplos de horas pedidos
			const unsigned int mult = block.size() / hours;
			//unsigned int count_blocks = 0;
			if(mult > 0)
			{
				Block::const_iterator it_hour = block.begin();
				Block block_mult;
				for(unsigned int i = 0; i < mult; i++)
				{
					block_mult.clear();
					for(unsigned int j = 0; j < hours; j++)
					{
						block_mult.push_back(*it_hour);
						it_hour++;
					}
					//count_blocks++;
					day.add(block_mult);
				}
				/*if(day.get_blocks().size() != count_blocks)		
				{
					std::string msg = "Hay '";
					msg += std::to_string(count_blocks) + "' bloques creados, sin embargo, se agregaron " + std::to_string(day.get_blocks().size());
					throw core::Exception(msg,__FILE__,__LINE__);
				}*/
				//si no es exacto, se pueden hacer otras combinaciones
				if(block.size() % hours > 0) combns(days,hours,block,day);
				continue;//estas horas ya fueron repartidas
			}
			else
			{
				
			}
		}
		//std::cout << "Day::combns - Step 4\n";
	}
	void Day::combns(std::list<Day>&, unsigned int hours, const Block& block, Day& day)const
	{
		unsigned int free = block.size() - ((block.size() / hours) * hours);
		const unsigned int mult = block.size() / hours;
		Block::const_iterator it_day = block.begin();
		
		Block block_mult;
		//comenzar enfrente
		for(unsigned int k = 0; k < free; k++)
		{
			if(it_day != block.end()) 
			{
				it_day++;
			}
			else
			{
				break;
			}
			for(unsigned int i = 0; i < mult; i++)
			{
				block_mult.clear();
				for(unsigned int j = 0; j < hours; j++)
				{
					if(it_day == block.end())
					{
						 break;
					}
					block_mult.push_back(*it_day);
					it_day++;
				}
				day.add(block_mult);
			}	
		}		
	}
	void Day::print_day(std::ostream& out) const
	{
		for(const core::DataTime& dt : *this)
		{
			dt.print(out,"%a %H:%M");
			out << " ";
		}
	}	
	void Day::print_blocks(std::ostream& out) const
	{		
		for(const Block& b : blocks)
		{
			out << "\t";
			for(const core::DataTime* dt : b)
			{
				dt->print(out,"%a %H:%M");
				out << " ";
			}
			out << "\n";
		}
	}	
	void Day::print_intevals_csv(std::ostream& out, const Data& data) const
	{
		if(not check()) throw core::Exception("El dia no es valido",__FILE__,__LINE__);
		
		for(unsigned int i = 0; i < size(); i++)
		{
			if(size() == 1)
			{
				
			}
			else
			{
			
			}
			if(i < size() - 1) out << ",";
		}
	}
	check_codes Day::check() const
	{
		if(size() == 0 and blocks.size() == 0)  return check_codes::PASS;
		
		//verificar ordenammiento
		/*const_iterator itPrev = begin();
		const_iterator itActual = begin();
		itActual++;
		while(itActual != end())
		{
			if((*itActual).tm_hour != (*itPrev).tm_hour + 1)
			{
				return false;
			}
			
			itActual++;
			itPrev++;			
		}*/

		for(const core::DataTime dt : *this)
		{
			if((*begin()).tm_wday != dt.tm_wday) return check_codes::HOURS_DIFFERENT_DAY;
		}
		
		unsigned int countHB = 0;
		for(const Block& b : blocks)
		{
			countHB += b.size();
		}
		if(countHB != size()) return check_codes::BLOCK_CONTENT_SIZE_FAIL;//no coinciden las cantidad de horas
		
		
		return check_codes::PASS;
	}
	const Day::Block* Day::random_block() const
	{
		if(blocks.size() == 0) return NULL;
		else if(blocks.size() == 1) return &blocks.front();
				
		std::uniform_int_distribution<> distrib(0, blocks.size());		
		Blocks::const_iterator it = blocks.begin();
		std::advance(it,distrib(gen));
		if(it != blocks.end()) &*it;

		return NULL;
	}
	void Day::get_hours_around(const core::DataTime& hour, unsigned int count,Day::Block& block)const
	{
		if(count == 0) return;
		else if(count == 1) 
		{
			get_hours_around(hour,block);
			return;
		}
		
		const_iterator it = std::find(begin(),end(),hour);
		if(it == end()) return;
		for(unsigned int i = 0; i < count; i++)
		{
			
		}
	}
	void Day::get_hours_around(const core::DataTime& hour,Day::Block& block)const
	{
		const_iterator it = std::find(begin(),end(),hour);
		if(it != end())
		{
			block.push_back(&*it);
		}
		else
		{
			return;
		}
	}
	/*bool Day::is_continue(const core::DataTime& first, const core::DataTime& second, const Data& data)
	{
		core::DataTime newdt;
		data.config.add(first,1,newdt);
		if(second == newdt) return true;
		
		return false;
	}*/

	
	WeekOptions::WeekOptions() : std::vector<DaysOptions>(WeekHours::WEEK_SIZE) 
	{
		
	}
	
	void WeekOptions::random(WeekHours& week)
	{
		for(unsigned int day_actual = 0; day_actual < WeekHours::WEEK_SIZE; day_actual++)
		{
			if(at(day_actual).size() == 0 ) continue;//si no hay elementosa en elk dia actual omitir
			
			std::uniform_int_distribution<> distrib(0, at(day_actual).size() - 1);
			DaysOptions::iterator it = at(day_actual).begin();
			std::advance(it,distrib(gen));
			week[day_actual] = *it;
		}
	}
	unsigned int WeekOptions::count()const
	{
		unsigned int totals = 0;
		unsigned int actual_combs;
		std::list<WeekHours>::iterator it_last;
		for(unsigned int day_actual = 0; day_actual < WeekHours::WEEK_SIZE; day_actual++)
		{
			if(at(day_actual).size() == 0 ) continue;//si no hay elementosa en elk dia actual omitir

			if(totals > 0) actual_combs = totals * at(day_actual).size();
			else actual_combs = at(day_actual).size();

			totals += actual_combs;
		}

		return totals;
	}
	/*bool WeekOptions::get_day(unsigned int day,unsigned int hours,const core::DataTime& base,Day& d)const
	{
		DaysOptions ops;
		for(unsigned int i = 0; i < at(day).size(); i++)
		{
			
		}
	}*/
	
	
	
	const unsigned int WeekHours::WEEK_SIZE = 7;
	WeekHours::WeekHours()
	{
		resize(7);
	}
	WeekHours& WeekHours::inters(const WeekHours& comp1, const WeekHours& comp2)
	{
		if(comp2.size() != comp1.size() and size() == comp1.size()) throw core::Exception("La cantidad de dias no coinciden",__FILE__,__LINE__);
		
		for(unsigned int i = 0; i < size(); i++)
		{
			at(i).inters(comp1[i],comp2[i]);
		}
		
		return *this;	
	}
	unsigned int WeekHours::days_disp() const
	{
		unsigned int count = 0;
		for(const Day& day : *this)
		{
			if(day.size() > 0) count++;
		}
		
		return count;
	}
	
	void WeekHours::combns(const Subject& subject, WeekOptions& week_combs)const
	{
		if(week_combs.size() != WEEK_SIZE)  throw core::Exception("La cantidad de dias en la semana incorrecto",__FILE__,__LINE__);
		if(size() != week_combs.size()) throw core::Exception("La cantidad de dias en la semana incorrecto",__FILE__,__LINE__);
		if(size() != WEEK_SIZE) throw core::Exception("La cantidad de dias en la semana incorrecto",__FILE__,__LINE__);
		
		
		/*
		unsigned int disp = days_disp();
		unsigned int mean_hours,diff_hours,botton_hours,floor_hours;
		mean_hours = subject.get_time() / disp;
		if(mean_hours == 0) 
		{
			mean_hours = 1;
			diff_hours = 0;
			botton_hours = 0;
			floor_hours = 0;
		}
		else
		{
			diff_hours = subject.get_time() - (mean_hours * disp);
			botton_hours = mean_hours + diff_hours;
			floor_hours = mean_hours - diff_hours;
		}
				
		for(unsigned int day = 0; day < size(); day++)
		{
			if(at(day).haveDisponible())
			{				
				at(day).combns(week_combs[day],mean_hours);
				if(botton_hours > mean_hours) at(day).combns(week_combs[day],botton_hours);
				if(floor_hours > 0) at(day).combns(week_combs[day],floor_hours);
			}
		}
		*/
		for(unsigned int day = 0; day < size(); day++)
		{
			for(unsigned int hours = 1; hours <= subject.get_time(); hours++)
			{
				if(at(day).haveDisponible()) at(day).combns(week_combs[day],hours);
			}
		}
		
		//TODO:Agregar combinacion para 'Menor cantidad de dias'
		//Dever dar la posibildad de la mayor cantidad de horas posibles al dia
		
		//TODO:Hay varias combinaciones posibles desde 1 hasta subject->get_time()
		
	}
	void WeekHours::sort()
	{
		for(Day& day : *this)
		{
			day.sort();
		}
	}
	void WeekHours::print(std::ostream& out) const
	{
		for(const Day& day : *this)
		{
			if(day.haveDisponible ())
			{
				out << "\n";
				day.print_day(out);
				out << "\n";
				day.print_blocks(out);
			}
		}
	}
	check_codes WeekHours::check() const
	{
		for(const Day& day : *this)
		{
			check_codes code = day.check();
			if(day.check() != check_codes::PASS) return code;
		}
		
		return check_codes::PASS;
	}
	bool WeekHours::empty()const
	{
		for(const Day& day : *this)
		{
			if(day.size() > 0) return false;
		}

		return true;
	}
	unsigned int WeekHours::count_hours()const
	{
		unsigned int count = 0;
		for(const Day& day : *this)
		{
			count += day.size();
		}
		
		return count;
	}
	void WeekHours::print_intevals_csv(std::ostream& out, const Data& data) const
	{
		for(unsigned int i = 0; i < size(); i++)
		{
			at(i).print_intevals_csv(out,data);
		}
	}
	void WeekHours::clear_days()
	{
		for(unsigned int i = 0; i < size(); i++)
		{
			at(i).clear();
		}
	}
	/*void WeekHours::cover(const Subject& subject, WeekHours& week)const
	{
		const Day* day = random_day_disp();
		if(day == NULL) return; //No hay dias disponibles
		
		const Day::Block* block = day->random_block();
		if(block == NULL) return;//No hay horas disponibles
		
		const core::DataTime* hour = *random(*block);
				
		unsigned int disp = days_disp();
		unsigned int hours_per_day = subject.get_time() / disp;
		unsigned int hours_hat = subject.get_time() - (hours_per_day * disp);
				
		for(unsigned int i = 0; i < disp; i++)
		{
			if(at(i).size() > 0)
			{
				Day::Block block;
				day->get_hours_around(*hour,hours_per_day,block);
				if(block.size() > 0) week[i].add(block);
				block.clear();
			}
		}					
	}*/
	const Day* WeekHours::random_day_disp()const
	{
		if(days_disp() == 0) return NULL;
		
		const Day* day = NULL;
		std::uniform_int_distribution<> distrib(0, 6);
		do
		{
			day = &at(distrib(gen));
		}
		while(day->size() > 0);
		
		return day;
	}


	
	
	
	
	

	Time::Time()
	{}
	Time::Time(const std::string& b,const std::string& e)
	{
		set_begin(b);
		set_end(e);
	}
	void Time::granulate(const Configuration* config, Day& out)
	{
		if(begin.tm_wday != end.tm_wday) throw core::Exception("El intervalo de tiempo deve especificar el mismos dia.",__FILE__,__LINE__);
		if(begin.tm_hour >= end.tm_hour) throw core::Exception("La hora de inicio deve ser meno que lahora final.",__FILE__,__LINE__);
		
		int hours = config->to_hours(begin.diff(end));	
		
		if(hours < 1) return;
		
		tm tm1 = begin;
		time_t t = mktime(&tm1);
		tm* newt = localtime(&t);
		newt->tm_wday = begin.tm_wday;
		out.push_back(*newt);
		//std::cout << "t = " << t << "\n";
		for(int i = 1; i < hours; i++)
		{
			t += config->get_time_per_hour() * 60; // 60 segundos por 60 minutos = una hora
			//std::cout << "t = " << t << "\n";
			newt = localtime(&t);
			newt->tm_wday = begin.tm_wday;
			out.push_back(*newt);
		}
	}
	void Time::granulate(const Configuration* config, WeekHours& out)
	{
		if(begin.tm_wday != end.tm_wday) throw core::Exception("El intervalo de tiempo deve especificar el mismos dia.",__FILE__,__LINE__);
		if(begin.tm_hour >= end.tm_hour) throw core::Exception("La hora de inicio deve ser meno que lahora final.",__FILE__,__LINE__);
		
		int hours = config->to_hours(begin.diff(end));	
		
		if(hours < 1) return;
		
		Day& day = out[begin.tm_wday];
		Day::Block b;
		day.get_blocks().push_back(b);
		Day::Block& block = day.get_blocks().back();
		
		tm tm1 = begin;
		time_t t = mktime(&tm1);
		tm* newt = localtime(&t);
		newt->tm_wday = begin.tm_wday;
		day.push_back(*newt);
		block.push_back(&day.back());
		//std::cout << "t = " << t << "\n";
		for(int i = 1; i < hours; i++)
		{
			t += config->get_time_per_hour() * 60; // 60 segundos por 60 minutos = una hora
			//std::cout << "t = " << t << "\n";
			newt = localtime(&t);
			newt->tm_wday = begin.tm_wday;
			day.push_back(*newt);
			block.push_back(&day.back());
		}
	}
	void Time::set_begin(const Configuration* config,const std::string& str)
	{
		strptime(str.c_str(), Configuration::formats_dt_dayn_hour.c_str(),&begin);
	}
	void Time::set_end(const Configuration* config,const std::string& str)
	{
		strptime(str.c_str(), Configuration::formats_dt_dayn_hour.c_str(),&end);
	}
	void Time::set_begin(const std::string& str)
	{
		strptime(str.c_str(), "%w %H:%M",&begin);
	}
	void Time::set_end(const std::string& str)
	{
		strptime(str.c_str(), "%w %H:%M",&end);
	}




	const std::string Configuration::formats_dt_hour = "%H:%M";	
	const std::string Configuration::formats_dt_day_hour = "%a %H:%M";
	const std::string Configuration::formats_dt_dayn_hour = "%w %H:%M";
	
	Configuration::Configuration()
	{
		schema_week = SchemaWeek::MS;
		time_per_hour = 60;
		format = FormatDT::DAY_HOUR;
	}
	
	unsigned int Configuration::get_time_per_hour() const
	{
		return time_per_hour;
	}
	Configuration::SchemaWeek Configuration::get_schema_week()const
	{
		return schema_week;
	}
	Configuration::Schema Configuration::get_schema()const
	{
		return schema;
	}
	/*const std::string& Configuration::get_format_string_datatime()const
	{
		switch(format)
		{
			case FormatDT::HOUR:
				return formats_dt_hour;
			case FormatDT::DAY_HOUR:
				return formats_dt_day_hour;
			case FormatDT::NONE:
				throw core::Exception("Formato de tiempo no asignado",__FILE__,__LINE__);
			default:
				throw core::Exception("Formato de tiempo desconocido",__FILE__,__LINE__);
		}
	}
	const std::string& Configuration::get_format_string_datatime(FormatDT format)
	{
		switch(format)
		{
			case FormatDT::HOUR:
				return formats_dt_hour;
			case FormatDT::DAY_HOUR:
				return formats_dt_day_hour;
			case FormatDT::NONE:
				throw core::Exception("Formato de tiempo no asignado",__FILE__,__LINE__);
			default:
				throw core::Exception("Formato de tiempo desconocido",__FILE__,__LINE__);
		}
	}*/
	void Configuration::set_schema(Schema s)
	{
		schema = s;
	}
	long Configuration::to_hours(double t)const
	{
		long mins = t/60.0;
		long hours = mins/time_per_hour;
		return hours;
	}
	int Configuration::get_begin_day() const
	{
		switch(schema_week)
		{
			case SchemaWeek::MF:
			case SchemaWeek::MS:
				return 1;
				
			default:
				throw core::Exception("Esquema de semana desconocido.",__FILE__,__LINE__);
		}
	}
	Configuration::FormatDT Configuration::get_format_dt()const
	{
		return format;
	}
	void Configuration::add(const core::DataTime& dt, unsigned int hours, core::DataTime& result)
	{
		tm tm_dt = dt;
		time_t dt_t = mktime(&tm_dt);
		dt_t += hours * get_time_per_hour() * 60;
		result = *localtime(&dt_t);
	}
	void Configuration::rest(const core::DataTime& dt, unsigned int hours, core::DataTime& result)
	{
		tm tm_dt = dt;
		time_t dt_t = mktime(&tm_dt);
		dt_t -= hours * get_time_per_hour() * 60;
		result = *localtime(&dt_t);
	}
	
	
	
	Target::Target() : config(NULL)
	{
		init();
	}
	Target::Target(const Configuration* c) : config(c)
	{
		init();
	}
	void Target::init()
	{
		times.resize(7);
	}
	const Configuration* Target::operator =(const Configuration* c)
	{
		config = c;
		return c;
	}
	const Configuration* Target::set(const Configuration* c)
	{
		config = c;
		return c;
	}
	/*void Target::save(const Day& day)
	{
		for(const core::DataTime& dt : day)
		{
			times[dt.tm_wday].push_back(dt);//el dia de la semana es el indice ene le arreglo
		}
	}*/
	WeekHours& Target::get_week()
	{
		return times;
	}
	const WeekHours& Target::get_week()const
	{
		return times;
	}
	void Target::print(std::ostream& out) const
	{
		for(const Day& day : times)
		{
			for(const core::DataTime& dt: day)
			{
				out << std::put_time(&dt, "%a %H:%M");
				out << " ";
			}
			out << ",";
		}
	}
	/*bool Target::cmpHour(const core::DataTime& f,const core::DataTime& s)
	{
		return f.tm_hour < s.tm_hour;
	}*/
	/*void Target::sort()
	{
		for(Day& day : times)
		{
			day.sort();
		}
	}*/
	
	
	

	Teacher::Teacher(const std::string& name,const std::string& ap,const std::string& am) : oct::core::Person(name,ap,am)
	{
		
	}
	Teacher::Teacher(const std::string& name) : oct::core::Person(name)
	{
		
	}
	Teacher::Teacher()
	{
		
	}
	void Teacher::print(std::ostream& out) const
	{
		out << get_name() << ",";
		Target::print(out);
	}



	
	
	Room::Room(const std::string& n)
	{
		name = n;
	}
	Room::Room()
	{
		
	}

	Room& Room::operator =(const std::string& n)
	{
		name = n;
		
		return *this;
	}
	const std::string& Room::get_name()const
	{
		return name;
	}
	void Room::print(std::ostream& out)const
	{
		out << get_name() << ",";
		Target::print(out);
	}
	
	

	
	Subject::Subject(const std::string& n)
	{
		name = n;
	}
	Subject::Subject()
	{
		
	}
	const std::string& Subject::get_name()const
	{
		return name;
	}
	void Subject::set(const std::string& n,unsigned int t)
	{
		name = n;
		time = t;
	}
	unsigned int Subject::get_time()const
	{
		return time;
	}
	void Subject::print(std::ostream& out) const
	{
		out << get_name() << ",";
		out << get_time();
	}
	
	
	
	
	Targets::Targets() : dataObject(NULL)
	{
	}
	Targets::Targets(const Data* d) : dataObject(d)
	{
	}		
	const Data* Targets::operator = (const Data* d)
	{
		dataObject = d;
		return d;
	}		
	const Data* Targets::set(const Data* d)
	{
		dataObject = d;
		return d;
	}
	void Targets::fetch_times(Target& target,std::stringstream& ssline, unsigned int line,const std::string& fn)
	{
		ec::sche::Time time;
		std::string data,strH;
		int timeDay = dataObject->config.get_begin_day();
		while(std::getline(ssline,data,','))
		{
			if(dataObject->config.get_format_dt() == Configuration::FormatDT::HOUR and timeDay > 6) 
			{
				std::string msg = "El formtato indicado solo permite 7 datos de entrada en ";
				msg += "'" + fn + "':" + std::to_string(line);
				throw core::Exception(msg, __FILE__,__LINE__);
			}
			std::stringstream ssTime(data);
			
			std::getline(ssTime,strH,'-');
			//std::cout << "strH : '" << strH << "'\n";
			if(dataObject->config.get_format_dt() == Configuration::FormatDT::HOUR) strH = std::to_string(timeDay) + " " + strH;
			time.set_begin(strH);
			//time.begin.print(std::cout,Configuration::formats_dt_day_hour);
			//std::cout << "\n";
			
			std::getline(ssTime,strH,'-');
			//std::cout << "strH : '" << strH << "'\n";
			if(dataObject->config.get_format_dt() == Configuration::FormatDT::HOUR) strH = std::to_string(timeDay) + " " + strH;
			time.set_end(strH);
			//time.end.print(std::cout,Configuration::formats_dt_day_hour);
			//std::cout << "\n";
			
			if(time.begin.tm_wday != time.end.tm_wday) 
			{
				std::string msg = "El intervalode tiempo se deve referir al mimso dia ";
				msg += "'" + fn + "':" + std::to_string(line);
				throw core::Exception(msg, __FILE__,__LINE__);
			}
			if(time.begin.tm_hour >= time.end.tm_hour) 
			{
				std::string msg = "La hora de inicio deve ser menor que la hora final ";
				msg += "'" + fn + "':" + std::to_string(line);
				throw core::Exception(msg, __FILE__,__LINE__);
			}
			time.granulate(&dataObject->config,target.get_week());
			timeDay++;
		}
	}
	
	
	
	


	
	Teachers::Teachers(const std::string& fn)
	{
		loadFile(fn);
	}
	Teachers::Teachers()
	{

	}
	const std::list<Teacher>& Teachers::get_list() const
	{
		return teachers;
	}
	void Teachers::loadFile(const std::string& fn)
	{
		if(not dataObject) throw core::Exception("dataObject no asignado.", __FILE__,__LINE__);
		
		oct::core::Shell shell;
		//std::cout << "cwd 2:" << shell.cwd() << "\n";
		//std::cout << "dir 2:" << fn << "\n";
		
		std::fstream csv(fn, std::ios::in);
		std::string line,data,strTime,strH;
		if(csv.is_open())
		{
			unsigned int line_number = 1;
			while(std::getline(csv,line))
			{
				std::stringstream str(line);
				std::getline(str,data,',');
				Teacher t;
				teachers.push_back(t);
				Teacher& teacher = teachers.back();
				((Target&)teacher) = &dataObject->config;
				//std::cout << data << ",";
				teacher = data;
				fetch_times(teacher,str,line_number,fn);
				line_number++;
				//std::cout << "\n";
			}
			indexing();
		}	
		else
		{
			std::string msg = "Fallo la aperturade '";
			msg += fn + "'";
			throw core::Exception(msg,__FILE__,__LINE__);
		}	
	}
	void Teachers::print(std::ostream& out)
	{
		for(Teacher& row : teachers)
		{
			row.print(out);
			out << "\n";
		}
	}	
	void Teachers::indexing()
	{
		if(teacher_by_name.size() > 0) teacher_by_name.clear();
		for(Teacher& t : teachers)
		{
			teacher_by_name.insert({t.get_name().c_str(),&t});
		}
	}
	const Teacher* Teachers::search(const std::string& str) const
	{
		std::map<std::string, Teacher*>::const_iterator it = teacher_by_name.find(str);

		if(it != teacher_by_name.end()) return it->second;
		return NULL;		
	}



	

	
		
	Subjects::Subjects(const std::string& fn, const Data* d) : Targets(d)
	{
		loadFile(fn);
	}
	Subjects::Subjects()
	{
	}
	void Subjects::loadFile(const std::string& fn)
	{
		if(not dataObject) throw core::Exception("dataObject no asignado.", __FILE__,__LINE__);
		
		std::fstream csv(fn, std::ios::in);
		std::string line,data,strH;
		if(csv.is_open())
		{
			while(std::getline(csv,line))
			{
				std::stringstream str(line);
				Subject s;
				subjects.push_back(s);
				Subject& subject = subjects.back();
				((Target&)subject) = &dataObject->config;
				//std::cout << line;
				//std::cout << data << ",";

				std::getline(str,data,',');
				//std::cout << data << ",";
				std::string name = data;	
				
				std::getline(str,data,',');
				std::string time = data;
				//std::cout << "\n";
				subject.set(name,std::stoi(time));
				/*if(dataObject->config.get_schema() == Configuration::Schema::WITH_SUBJECTS_TIMES)
				{
					ec::sche::Time time;
					int timeDay = dataObject->config.get_begin_day();
					while(std::getline(str,data,','))
					{
						std::stringstream ssTime(data);
						std::getline(ssTime,strH,'-');
						if(dataObject->config.get_format_dt() == Configuration::FormatDT::HOUR) strH = std::to_string(timeDay) + " " + strH;
						time.set_begin(strH);
						std::getline(ssTime,strH,'-');
						if(dataObject->config.get_format_dt() == Configuration::FormatDT::HOUR) strH = std::to_string(timeDay) + " " + strH;
						time.set_end(strH);
						time.granulate(&dataObject->config,subject.get_times());
						timeDay++;
					}
				}	*/			
			}
			indexing();
		}
		else
		{
			std::string msg = "Fallo la aperturade '";
			msg += fn + "'";
			throw core::Exception(msg,__FILE__,__LINE__);
		}		
	}
	void Subjects::print(std::ostream& out)const
	{
		for(const Subject& s : subjects)
		{
			s.print(out);
			out << "\n";
		}
	}		
	void Subjects::indexing()
	{
		if(subject_by_name.size() > 0) subject_by_name.clear();
		for(Subject& s : subjects)
		{
			subject_by_name.insert({s.get_name().c_str(),&s});
		}
	}
	const Subject* Subjects::search(const std::string& str) const
	{
		std::map<std::string, Subject*>::const_iterator it = subject_by_name.find(str);

		if(it != subject_by_name.end()) return it->second;
		return NULL;		
	}
	const std::list<Subject>& Subjects::get_list() const
	{
		return subjects;
	}
	
	
	
	
	
	



	Teachers_Subjects::Row::Row()
	{
		
	}	
	void Teachers_Subjects::Row::print(std::ostream& out)const
	{
		out << teacher->get_name() << ",";
		for(const Subject* s : *this)
		{
			out << s->get_name() << ",";
		}
	}
	Teachers_Subjects::Teachers_Subjects(const std::string& fn,const Data* d) : Targets(d)
	{
		loadFile(fn);
	}
	Teachers_Subjects::Teachers_Subjects()
	{

	}	
	const std::list<Teachers_Subjects::Row>& Teachers_Subjects::get_list() const
	{
		return teachers_subjects;
	}
	const std::map<std::string, Teachers_Subjects::HBS>& Teachers_Subjects::get_hbs()const
	{
		return hbs_by_subject;
	}
	void Teachers_Subjects::loadFile(const std::string& fn)
	{
		
		if(not dataObject) throw core::Exception("dataObject no asignado.", __FILE__,__LINE__);
		
		std::fstream csv(fn, std::ios::in);
		std::string line,data;
		if(csv.is_open())
		{
			while(std::getline(csv,line))
			{
				std::stringstream str(line);
				//std::cout << line;
				std::getline(str,data,',');
				Teachers_Subjects::Row row;
				const Teacher* rt = dataObject->teachers.search(data);
				if(rt)
				{
					row.teacher = rt;
				}
				else 	
				{
					std::string msg = "Archivo '";
					msg += fn + "', el maestro '" + data + "', no esta registrada en su correpondiente archivo.";
					throw core::Exception(msg,__FILE__,__LINE__);
				}
				//std::cout << data << ",";
				while(std::getline(str,data,','))
				{
					//std::cout << data << ",";				
					const Subject* rs = dataObject->subjects.search(data);
					if(rs)
					{
						row.push_back(rs);
					}
					else 	
					{
						std::string msg = "Archivo '";
						msg += fn + "', la materia '" + data + "', no esta registrada en su correpondiente archivo.";
						throw core::Exception(msg,__FILE__,__LINE__);
					}
				}
								
				//std::cout << "\n";
				teachers_subjects.push_back(row);
			}
			indexing();
		}	
		else
		{
			std::string msg = "Fallo la aperturade '";
			msg += fn + "'";
			throw core::Exception(msg,__FILE__,__LINE__);
		}
	}
	void Teachers_Subjects::print(std::ostream& out)const
	{
		for(const Row& row : teachers_subjects)
		{
			row.print(out);
			out << "\n";
		}
	}
	const Teachers_Subjects::Row* Teachers_Subjects::searchTeachers(const std::string& str)const
	{
	 	auto it = teachers_by_name.find(str);
	 	if(it != teachers_by_name.end()) return it->second;
	 	return NULL;
	}
	void Teachers_Subjects::searchSubjects(const std::string& str, std::list<const Row*>& l)const
	{
		typedef std::multimap<std::string, Row*>::const_iterator iterator;
		std::pair<iterator,iterator> result = subjects_by_name.equal_range(str);
		for(iterator it = result.first; it != result.second; it++)
		{
			l.push_back(it->second);
		}
	}	
	void Teachers_Subjects::indexing()
	{
		if(teachers_by_name.size() > 0) teachers_by_name.clear();
		if(subjects_by_name.size() > 0) subjects_by_name.clear();
		for(Row& row : teachers_subjects)
		{
			if(not row.teacher) throw core::Exception("Valor nulo para puntero de Maestro",__FILE__,__LINE__);
			
			auto itTeacher = teachers_by_name.find(row.teacher->get_name());
			if(itTeacher != teachers_by_name.end()) 
			{
				std::string msg = "El Maestro '";
				msg += row.teacher->get_name() + "' ya esta registrado previeamente en el artivo Maestros/Materias.";
				throw core::Exception(msg,__FILE__,__LINE__);
			}
			teachers_by_name.insert({row.teacher->get_name(),&row});
			
			for(const Subject* subject : row)
			{
				if(not subject) throw core::Exception("Valor nulo para puntero de Materia",__FILE__,__LINE__);
				subjects_by_name.insert({subject->get_name(),&row});				
				
				std::map<std::string, HBS>::iterator it = hbs_by_subject.find(subject->get_name());
				if(it == hbs_by_subject.end())
				{		
					HBS hbs;
					hbs.subject = subject;
					hbs.row = &row;
					hbs.req_hours = subject->get_time();
					hbs.disp_hours = row.teacher->get_week().count_hours();
					hbs_by_subject.insert({subject->get_name(),hbs});					
				}
				else
				{
					HBS& hbs = (*it).second;
					hbs.disp_hours += row.teacher->get_week().count_hours();
				}				
			}
		}
	}
	void Teachers_Subjects::hour_by_subjects_csv(std::ofstream& fn)
	{
		std::map<std::string, HBS> hbs_list;
		HBS hbs;
		std::list<const Row*> rows;		
		for(const Subject& subject : dataObject->subjects.get_list())
		{
			searchSubjects(subject.get_name(), rows);
			for(const Row* row : rows)
			{
				//hbs_list.find();
			}
			
			rows.clear();
		}
	}
	
	
	Rooms::Rooms(const std::string& fn)
	{
		loadFile(fn);
	}
	Rooms::Rooms()
	{
		
	}	
	const std::list<Room>& Rooms::get_list() const
	{
		return rooms;
	}
	
	void Rooms::loadFile(const std::string& fn)
	{		
		if(not dataObject) throw core::Exception("dataObject no asignado.", __FILE__,__LINE__);
		
		std::fstream csv(fn, std::ios::in);
		std::string line,data,strTime,strH;
		if(csv.is_open())
		{
			unsigned int line_number = 1;
			while(std::getline(csv,line))
			{
				std::stringstream str(line);
				std::getline(str,data,',');
				Room r;
				rooms.push_back(r);	
				Room& room = rooms.back();
				//std::cout << data << ",";
				room = data;
				//std::getline(str,data,',');
				//row.subject = data;				
				fetch_times(room,str,line_number,fn);
				line_number++;
				//std::cout << "\n";
			}
			indexing();
		}	
		else
		{
			std::string msg = "Fallo la aperturade '";
			msg += fn + "'";
			throw core::Exception(msg,__FILE__,__LINE__);
		}	
	}	
	void Rooms::print(std::ostream& out)const
	{
		for(const Room& r : rooms)
		{
			r.print(out);
			out << "\n";
		}
	}
	const Room* Rooms::search(const std::string& str) const
	{
		std::map<std::string, Room*>::const_iterator it = rooms_by_name.find(str);

		if(it != rooms_by_name.end()) return it->second;
		return NULL;		
	}
	void Rooms::indexing()
	{
		if(rooms_by_name.size() > 0) rooms_by_name.clear();
		for(Room& r : rooms)
		{
			rooms_by_name.insert({r.get_name(),&r});
		}
	}



	Group::Group()
	{
	}		
	void Group::print(std::ostream& out)const
	{
		out << room->get_name() << ",";
		//out << teacher.get_name() << ",";
	}
	
	Groups::Groups(const std::string& fn,const Data* d) : Targets(d)
	{
		loadFile(fn);
	}
	Groups::Groups()
	{
		
	}	
	const std::list<Group>& Groups::get_list() const
	{
		return groups;
	}
	const std::map<Groups::key_hbs, Groups::HBRS>& Groups::get_hbrs()const
	{
		return hbrs_list;
	}
	
	void Groups::loadFile(const std::string& fn)
	{
		
		if(not dataObject) throw core::Exception("dataObject no asignado.", __FILE__,__LINE__);
		
		std::fstream csv(fn, std::ios::in);
		std::string line,data,strTime,strH;
		if(csv.is_open())
		{
			while(std::getline(csv,line))
			{
				std::stringstream str(line);
				std::getline(str,data,',');
				Group row;
				//std::cout << "room : " << data << "\n";
				const Room* newr = dataObject->rooms.search(data);
				if(newr) 
				{
					row.room = newr;
				}
				else 	
				{
					std::string msg = "Archivo '";
					msg += fn + "', la materia '" + data + "', no esta registrada en su correpondiente archivo.";
					throw core::Exception(msg,__FILE__,__LINE__);
				}	
				
				while(std::getline(str,data,','))
				{	
					//std::cout << data << ",";
					const Subject* news = dataObject->subjects.search(data);	
					if(news) 
					{
						row.push_back(news);
					}
					else 	
					{
						std::string msg = "Archivo '";
						msg += fn + "', la materia '" + data + "', no esta registrada en su correpondiente archivo.";
						throw core::Exception(msg,__FILE__,__LINE__);
					}		
				}
				groups.push_back(row);
			}
			indexing();
			max_lessons = (*groups.begin()).size();
			for(std::list<Group>::iterator it = groups.begin(); it != groups.end(); it++)
			{
				if((*it).size()> max_lessons) max_lessons = (*it).size();
			}
		}	
		else
		{
			std::string msg = "Fallo la aperturade '";
			msg += fn + "'";
			throw core::Exception(msg,__FILE__,__LINE__);
		}	
	}	
	void Groups::print(std::ostream& out)const
	{
		for(const Group& row : groups)
		{
			row.print(out);
			out << "\n";
		}
	}
	const Group* Groups::search_name(const std::string& str) const
	{
		std::map<std::string, Group*>::const_iterator it = groups_by_name.find(str);

		if(it != groups_by_subject.end()) return it->second;
		return NULL;		
	}	
	const Group* Groups::search_by_subject(const std::string& str) const
	{
		std::map<std::string, Group*>::const_iterator it = groups_by_subject.find(str);

		if(it != groups_by_name.end()) return it->second;
		return NULL;		
	}
	void Groups::indexing()
	{
		if(groups_by_name.size() > 0) groups_by_name.clear();
		if(groups_by_subject.size() > 0) groups_by_subject.clear();
		std::map<std::string, Group*>::const_iterator it;
		for(Group& g : groups)
		{
			it = groups_by_name.find(g.room->get_name());
			//std::cout << "Grupo : " << g.room->get_name() << "\n";
			//std::cout << "Size : " << groups_by_name.size() << "\n";
			if(it != groups_by_name.end())
			{
				std::string msg;
				msg = "Se esta repitiendo el gruopo '" + g.room->get_name() + "'";
				throw core::Exception(msg,__FILE__,__LINE__);
			}
			groups_by_name.insert({g.room->get_name(),&g});
			for(const Subject* s : g)
			{
				groups_by_subject.insert({s->get_name(),&g});
				
				//std::cout << "Subject : " << s->get_name() << "\n";
				key_hbs key;
				key.room = g.room;
				key.subject = s;
				//std::cout << "find  " << s->get_name() << " - " << g.room->get_name() << ".\n";
				std::map<key_hbs, HBRS>::iterator it = hbrs_list.find(key);
				if(it == hbrs_list.end())
				{		
					//std::cout << "prev " << s->get_name() << " - " << g.room->get_name() << ".\n";
					HBRS hbrs;
					hbrs.disp_hours = 0;
					hbrs.subject = s;
					hbrs.room = g.room;
					hbrs.group = &g;
					WeekHours week;
					std::list<const Teachers_Subjects::Row*> rows;
					dataObject->teachers_subjects.searchSubjects(s->get_name(),rows);
					if(rows.empty()) 
					{
						std::string msg;
						msg = "No se encontro la amateria '" + s->get_name() + "' para el salon '" + g.room->get_name() + "'\n";;
						throw core::Exception(msg,__FILE__,__LINE__);
					}
					for(const Teachers_Subjects::Row* row : rows)
					{
						//std::cout << "comparando horaio " << s->get_name() << " - " << g.room->get_name() << " horaios\n";
						week.inters(g.room->get_week(),row->teacher->get_week());
						hbrs.disp_hours += week.count_hours();
						week.clear_days();
					}
					//std::cout << "inserting  " << s->get_name() << " - " << g.room->get_name() << ".\n";
					hbrs_list.insert({key,hbrs});				
				}
				else
				{
					HBRS& hbrs = (*it).second;
					WeekHours week;
					std::list<const Teachers_Subjects::Row*> rows;
					dataObject->teachers_subjects.searchSubjects(s->get_name(),rows);
					for(const Teachers_Subjects::Row* row : rows)
					{
						week.inters(g.room->get_week(),row->teacher->get_week());
						hbrs.disp_hours += week.count_hours();
						week.clear_days();
					}
				}
			}
		}
	}
	unsigned int Groups::get_max_lessons()const
	{
		return max_lessons;
	}
	bool Groups::key_hbs::operator < (const key_hbs& o) const
	{
		if(room->get_name() < o.room->get_name()) return true;
		else if(subject->get_name() < o.subject->get_name()) return true;
		return false;
	}
	
	
	
	void Data::load(const std::string& dir)
	{
		//TODO:validacion estricta delas entredas
		((Targets&)teachers) = this;
		teachers.loadFile(dir + "/teachers.csv");
		((Targets&)subjects) = this;
		subjects.loadFile(dir + "/subjects.csv");
		((Targets&)rooms) = this;
		rooms.loadFile(dir + "/rooms.csv");
		//
		((Targets&)teachers_subjects) = this;
		teachers_subjects.loadFile(dir + "/teachers-subjects.csv");
		((Targets&)groups) = this;
		groups.loadFile(dir + "/groups.csv");
	}
	
	
	
	Lesson::Lesson()
	{
		group = NULL;
		subject = NULL;
		teacher = NULL;
		room = NULL;
		data = NULL;
	}
	
	
	ClassRoom::ClassRoom()
	{

	}
	ClassRoom::ClassRoom(unsigned int z) : std::vector<Lesson>(z)
	{
		
	}
	ClassRoom::ClassRoom(const ClassRoom& g)
	{
		resize(g.size());

		for(unsigned int i = 0; i < g.size(); i++)
		{
			at(i) = g[i];
		}
	}
	ClassRoom& ClassRoom::operator =(const ClassRoom& g)
	{
		resize(g.size());
		
		for(unsigned int i = 0; i < g.size(); i++)
		{
			at(i) = g[i];
		}

		return *this;
	}
	void ClassRoom::juncting(const ClassRoom& g1,const ClassRoom& g2)
	{
		if(g1.size() != g2.size()) throw core::Exception("EL tamano de los registros no coincide.",__FILE__,__LINE__);
		if(g1.size() == 0) throw core::Exception("No hay lecciones",__FILE__,__LINE__);
		if(g2.size() == 0) throw core::Exception("No hay lecciones",__FILE__,__LINE__);
		
		resize(g1.size());
		std::bernoulli_distribution distrib(0.6);
		for(unsigned int i = 0; i < g1.size(); i++)
		{//selecciona al azar una de las collecciones para elegir agregar dico goal a la collacion actual
			if(distrib(gen)) 
			{
				at(i) = g1.at(i);
			}
			else 
			{
				at(i) = g2.at(i);
			}
		}
	}	
	void ClassRoom::mutate()
	{
		if(size() == 0) throw core::Exception("Hoario vacio",__FILE__,__LINE__);
		//std::cout << "\tLessons::mutate Step 1\n";
		
		std::uniform_int_distribution<int> distrib(0, size() - 1);
		std::bernoulli_distribution random_mutation(0.5);
		//std::cout << "\tLessons::mutate Step 2.0.0\n";
		if(random_mutation(gen))
		{
			//std::cout << "\tLessons::mutate Step 2.0.1\n";
			Lesson* lesson = &at(distrib(gen));
			if(not lesson->data)
			{
				throw core::Exception("No se asigno el objeto de datos",__FILE__,__LINE__);
			}
			//std::cout << "\tLessons::mutate Step 2.0.2\n";
			std::list<const Teachers_Subjects::Row*> rows;
			//std::cout << "\tMateria : " << lesson->subject->get_name() <<  " \n";
			lesson->data->teachers_subjects.searchSubjects(lesson->subject->get_name(),rows);
			if(rows.empty())
			{
				std::string msg = "No hay opciones en la busqueda de '";
				msg += lesson->subject->get_name() + "'";
				throw core::Exception(msg,__FILE__,__LINE__);
			}
			//std::cout << "\tLessons::mutate Step 2.0.3\n";
			//std::cout << lessons[subject].subject->get_name();
			std::list<const Teachers_Subjects::Row*>::const_iterator itr = random(rows);
			//std::cout << "\tLessons::mutate Step 2.0.4\n";
			if(itr == rows.end())
			{
				std::string msg = "No se encontro maestro asociado para '";
				msg += lesson->subject->get_name() + "'";
				throw core::Exception(msg,__FILE__,__LINE__);
			}
			//std::cout << "\tLessons::mutate Step 2.0.5\n";
			lesson->teacher = (*itr)->teacher;
			
			WeekHours week;
			WeekOptions week_opt;
			week.inters(lesson->room->get_week (),lesson->teacher->get_week());
			week.combns(*lesson->subject,week_opt);
			week_opt.random(lesson->week);
		}
		else
		{		
			//std::cout << "\tLessons::mutate Step 2.0.2\n";
			Lesson* lesson = &at(distrib(gen));
				
			WeekHours week;
			WeekOptions week_opt;
			week.inters(lesson->room->get_week(),lesson->teacher->get_week());
			week.combns(*lesson->subject,week_opt);
			week_opt.random(lesson->week);
		}
	}
	
	
	
	
	
	
	
	Schedule::Schedule()
	{

	}
	Schedule::Schedule(unsigned int z) : std::vector<ClassRoom>(z)
	{
		
	}
	Schedule::Schedule(const Schedule& g)
	{
		resize(g.size());

		for(unsigned int i = 0; i < g.size(); i++)
		{
			at(i) = g[i];
		}
		indexing();
	}
	Schedule& Schedule::operator =(const Schedule& g)
	{
		resize(g.size());
		
		for(unsigned int i = 0; i < g.size(); i++)
		{
			at(i) = g[i];
		}
		indexing();

		return *this;
	}
	void Schedule::indexing()
	{
		if(lesson_by_teacher.size() > 0) lesson_by_teacher.clear();
		for(const ClassRoom& lessons : *this)
		{
			for(const Lesson& g : lessons)
			{
				lesson_by_teacher.insert({g.teacher->get_name(),&g});
				//std::cout << "Indexing : " << g.teacher->get_name() << "\n";
			}
		}
	}
	void Schedule::search_teachers(const std::string& str, std::vector<const Lesson*>& result)const
	{
		typedef std::multimap<std::string, const Lesson*>::const_iterator iterator;
		std::pair<iterator,iterator> r = lesson_by_teacher.equal_range(str);
		for(iterator it = r.first; it != r.second; it++)
		{
			result.push_back(it->second);
		}
	}
	void Schedule::juncting(const Schedule& s1,const Schedule& s2)
	{
		//std::cout << "Size 1 = " << s1.size() << "\n";
		//std::cout << "Size 2 = " << s2.size() << "\n";
		if(s1.size() != s2.size()) throw core::Exception("Los tamanos de horaios no coincide",__FILE__,__LINE__);
		if(s1.size() == 0) throw core::Exception("Hoario vacio",__FILE__,__LINE__);
		if(s2.size() == 0) throw core::Exception("Hoario vacio",__FILE__,__LINE__);
		
		for(unsigned int i = 0; i < size(); i++)
		{
			at(i).juncting(s1[i],s2[i]);
		}
	}
	void Schedule::mutate()
	{
		if(size() == 0) throw core::Exception("Hoario vacio",__FILE__,__LINE__);
		std::uniform_int_distribution<int> distrib(0, size() - 1);
		at(distrib(gen)).mutate();
	}
		
}

