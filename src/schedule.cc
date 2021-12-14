

#include "schedule.hh"


namespace oct::core
{

}


namespace oct::ec::sche
{
	
	

	Single::Single(ID id,Enviroment& env,const Junction& j) : ec::Single(id,env,j)
	{

	}
	Single::Single(ID id,Enviroment& env, const Schedule& s) : ec::Single(id,env), schedule(s)
	{

	}
	/**
	** Criterios:
	** 		El mismo maestro no puede tener clases diferentes a la misma hora : No traslapes de Maestro
	** 		Dos clases no pueden tener el mismo horario en el mismo salon : No traslape de clase
	** 		La misma materia no puede tener diferentes maestros para un grupo: Maestro unico por materia
	** 		Los criterio de disponibilidad para cada objetivo dever ser compidos : disponibilidad
	** 		Hay criterios adicionales expresados en la configuracion que deve ser cumplidos
	**/
	void Single::eval()
	{
		
	}
	void Single::save(std::ofstream& fn)
	{

	}
	Population Single::juncting(std::list<oct::ec::Single*>& chils,const oct::ec::Single* single)
	{
		Population count = 0;
		
		for(ec::geneUS i = 0; i < getJunction().get_number(); i++,count++)
		{
			double randJ = core::randNumber();
			const Junction* juntion;
			if(randJ < 0.5) juntion = &getJunction();
			else juntion = &single->getJunction();
			Single* newsingle = new Single(env->nextID(),(Enviroment&)*env,*juntion);
			newsingle->schedule.juncting(&schedule,&((Single*)single)->schedule);
						
			chils.push_back(newsingle);
		}
					
		return count++;	
	}
	void Single::print(std::ostream&) const
	{

	}
	
	
	
	
	
	


Enviroment::Enviroment(const std::string& log,const std::string& dir)
{
	init();
	logDirectory = log;
	directory = dir;	
	data.load(directory);
	
	if(not shell.exists(logDirectory)) 
	{
		shell.mkdir(logDirectory);
		if(not shell.exists(logDirectory))
		{
			std::string msg = "El directorio '";
			msg += logDirectory + "' no existe.";
			throw oct::core::Exception(msg,__FILE__,__LINE__);
		}
	}
}

Enviroment::~Enviroment()
{
	init();
}
void Enviroment::init()
{
	initPopulation = 36;
	maxPopulation = initPopulation;
	maxProgenitor = initPopulation;
}



void Enviroment::initial()
{
	Schedules inits;
	inits.resize(initPopulation);
	
	if(initPopulation < data.groups.get_list().size() * 2) core::Exception("El tamano de la poblacion inicial es muy bajo",__FILE__,__LINE__);
	if((initPopulation % data.groups.get_list().size()) != 0 and (initPopulation / data.groups.get_list().size() ) < 2 ) throw core::Exception("La poblacion inicial deve ser multiplos de la cantida de grupos.",__FILE__,__LINE__);
	
	//
	for(Schedule& sche : inits)
	{
		Goal goal;
		for(Groups::const_iterator itGroup = data.groups.get_list().begin(); itGroup != data.groups.get_list().end(); itGroup++)
		{
			goal.group = &*itGroup;
			goal.room = (&*itGroup)->room;		
			for(const Subject* subjectGroup : *itGroup)
			{
				List<const Teachers_Subjects::Row*> rows;
				data.teachers_subjects.searchSubjects(subjectGroup->get_name(),rows);
				goal.subject = subjectGroup;
				const Teachers_Subjects::Row* r = rows.rand();
				if(r) goal.teacher = r->teacher;
				else 
				{
					std::string msg = "No se encontro maestro asociado para '";
					msg += subjectGroup->get_name() + "'";
					throw core::Exception(msg,__FILE__,__LINE__);
				}
				
				sche.push_back(goal);//todos los individuos deven tener los datos en el mismo orden, es importante para las operacionde apareo
			}
		}
	}
	
	//creando individuuos
	for(const Schedule& s : inits)
	{
		Single* single = new Single(nextID(),*this,s);
		push_back(single);
	}
}


/*
unsigned int Enviroment::counter()const
{
	unsigned int count = 0;
	for(Groups::const_iterator itGroup = data.groups.get_list().begin(); itGroup != data.groups.get_list().end(); itGroup++)
	{
		//goal.group = &*itGroup;
		//goal.room = (&*itGroup)->room;
		for(unsigned int j = 0; j < data.groups.get_list().size(); j++)
		{
			for(const Subject* subject : *itGroup)
			{
				std::list<const Teachers_Subjects::Row*> rows;
				data.teachers_subjects.searchSubjects(subject->get_name(),rows);
				for(const Teachers_Subjects::Row* ts : rows)//cada mestro
				{
					//goal.teacher = ts->teacher;
					for(const Subject* subject : *ts)
					{
						//goal.subject = subject;
												
						//const WeekHours& dispTeacher = ts->teacher->get_times();//disponibilidad de mestros
						//const WeekHours& dispRoom = (*itGroup).room->get_times();//disponibilidad de salon
						//const WeekHours& dispSubject = (*itGroup).room->get_times();//disponibilidad de salon
						//if(dispTeacher.size() != dispRoom.size()) throw core::Exception("La cantidad de dias no coinciden",__FILE__,__LINE__);
						count++; 
					}
				}
			}
		}
	}
}
*/
unsigned int Enviroment::counter()const
{
	unsigned int count = 0;
	for(Groups::const_iterator itGroup = data.groups.get_list().begin(); itGroup != data.groups.get_list().end(); itGroup++)
	{
			for(const Subject* subjectGroup : *itGroup)
			{
				List<const Teachers_Subjects::Row*> rows;
				data.teachers_subjects.searchSubjects(subjectGroup->get_name(),rows);
				for(const Teachers_Subjects::Row* ts : rows)
				{
					for(const Subject* subject : *ts)
					{
						count++; 
					}
				}
			}
	}
	
	return count;
}
void Enviroment::juncting()
{
	Single *single1,*single2;
	std::cout << "Enviroment::juncting()\n";
	do
	{
		ec::Single* single1 = getRandomSingle();
		if(single1 == NULL) continue;
		ec::Single* single2 = getRandomSingle();
		if(single2 == NULL) continue;		
		if(single1 == single2) continue;
		
		single1->juncting(newschils,single2);
	}
	while(newschils.size() + size() <= maxPopulation);
}

}

