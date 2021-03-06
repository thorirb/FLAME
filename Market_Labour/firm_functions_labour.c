#include "../header.h"
#include "../firm_agent_header.h"
#include "../library_header.h"

/*
 * \fn: int firm_labour_workforce_needed()
 * \brief:  A firm computes required number of employees to fulfill production
 * requirements.
 */
int firm_labour_workforce_needed()
{
    /* Making sure that at least one employee per firm is hired. */
    if (NO_EMPLOYEES < 1) {
        EMPLOYEES_NEEDED = 1;
    }
    
    VACANCIES = EMPLOYEES_NEEDED - NO_EMPLOYEES;
    
    if (PRINT_DEBUG_MODE) {
      printf("Firm %d is at Labour Market VACANCIES is %d\n", ID, VACANCIES);
    }
    
    return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_labour_fire()
 * \brief:  A firm picks a number of employees to be fired. And sends out a
 * notification message.
 */
int firm_labour_fire()
{
    /* Algorithm:
       Determine number of employees to be fired.
       Make a deterministic sampling. Pick the ones with higher indices.
       Send out fired message.
     */
    
    int n_to_fire, i, index;
    
    n_to_fire = NO_EMPLOYEES - EMPLOYEES_NEEDED;
    
    /* Keep at least one employee at the firm. */
    if (n_to_fire == NO_EMPLOYEES) {
        n_to_fire -= 1;
    }
    
    for (i = 0; i < n_to_fire; i++) {
        index = EMPLOYEES.size - 1;
        add_fired_message(EMPLOYEES.array[index]);
        remove_int(&EMPLOYEES, index);
    }
    
    NO_EMPLOYEES = EMPLOYEES.size;
    VACANCIES = 0;
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_labour_job_announcement_stage1()
 * \brief: a firm announces a number of vacancies.
 * a vacancy message holds firm id and wage to be paid.
 */
int firm_labour_job_announcement_stage1()
{
    if (WAGE_OFFER <= AVERAGE_WAGE) {
        WAGE_OFFER = 1.01 * AVERAGE_WAGE;
    }
    
    for (int i = 0; i < VACANCIES; i++) {
        add_vacancy_stage1_message(ID, WAGE_OFFER);
    }
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_labour_job_offer()
 * \brief: the firm recieves job application messages. Messages are sorted
 * by employee ids. Applicants with lower indices (employee ids) are hired.
 * This leads to a type of deterministic sampling. An employee applies to a
 * post one at a time.
 */
int firm_labour_job_offer_stage1()
{
    int n_hired, new_employee;

    /* Recieve job application messages. */
    n_hired = 0;
    
    START_JOB_MATCH_STAGE1_MESSAGE_LOOP
    new_employee = job_match_stage1_message->employee_id;
    add_int(&EMPLOYEES, new_employee);
    n_hired +=1;
	FINISH_JOB_MATCH_STAGE1_MESSAGE_LOOP
    
    /* It is possible that fewer than needed applications were recieved. */
    VACANCIES -= n_hired;
    NO_EMPLOYEES = EMPLOYEES.size;
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_labour_update()
 * \brief: the firm recieves job resignation message.
 * Labour force status is updated. Messages addressed to this agent
 * is filtered before hand.
 */
int firm_labour_update()
{
    int id_resigned, n_resigned, index, i;
    
    n_resigned = 0;
    /* Recieve job resignation messages addressed to the firm. */
    START_JOB_CHANGE_MESSAGE_LOOP
    id_resigned = job_change_message->employee_id;
    n_resigned++;
    index = EMPLOYEES.size - 1;
    for (i=0; i<EMPLOYEES.size; i++) {
        if (EMPLOYEES.array[i] == id_resigned) {
            index = i;
            break;
        }
    }
    remove_int(&EMPLOYEES, index);
	FINISH_JOB_CHANGE_MESSAGE_LOOP
    
    NO_EMPLOYEES = EMPLOYEES.size;
    VACANCIES += n_resigned;

 
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_labour_job_announcement_stage2()
 * \brief: a firm announces a number of unfilled or resigned vacancies.
 * a vacancy message holds firm id and wage to be paid. The behaviour is
 * activated only when there are still vacancies.
 */
int firm_labour_job_announcement_stage2()
{
    for (int i = 0; i < VACANCIES; i++) {
        add_vacancy_stage2_message(ID,WAGE_OFFER);
    }
    
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_labour_job_offer_stage2()
 * \brief: the firm recieves job application messages in the second round.
 * Messages are sorted by employee ids. Applicants with lower indices 
 * (employee ids) are hired.
 * This leads to a type of deterministic sampling. An employee applies to a
 * post one at a time.
 */
int firm_labour_job_offer_stage2()
{
    int n_hired, candidate;
    
    /* Recieve job application messages. */
    n_hired = 0;
    
    START_JOB_MATCH_STAGE2_MESSAGE_LOOP
    candidate = job_match_stage2_message->employee_id;
    add_int(&EMPLOYEES, candidate);
    n_hired++;
	FINISH_JOB_MATCH_STAGE2_MESSAGE_LOOP
    
    VACANCIES -= n_hired;
    NO_EMPLOYEES = EMPLOYEES.size;
    
    if (PRINT_DEBUG_MODE){
        printf("Firm ID = %d is at the end of Labour Market Stage 2: Number of Employees =%d, Vacancies = %d \n", ID, NO_EMPLOYEES, VACANCIES);
    }
	return 0; /* Returning zero means the agent is not removed */
}



/*
 * \fn: int firm_labour_pay_wages()
 * \brief: a firm pays wages and taxes on wages.
 */
int firm_labour_pay_wages()
{
    double payrolls;
    double labour_tax = 0;
 
    add_firm_household_wage_payment_message(ID, WAGE_OFFER);
    
    payrolls = (double)(WAGE_OFFER * NO_EMPLOYEES);
    labour_tax = payrolls * LABOUR_TAX_RATE;
    add_labour_tax_message(labour_tax);
    LIQUIDITY -= payrolls;
    LABOUR_COSTS += payrolls;
    
    
	return 0; /* Returning zero means the agent is not removed */
}


/*
 * \fn: int firm_labour_trace_wages()
 * \brief: The firm traces wages in the market and update.
 */
int firm_labour_trace_wages(){
    int unemployed = 0;
    double total_wages = 0;
    int total = 0;
    int id;
    START_EMPLOYMENT_STATUS_MESSAGE_LOOP
    id = employment_status_message->employer_id;
    if (id == 0) {
        unemployed++;
    }
    else{
        total_wages += employment_status_message->wage;
    }
    total++;
    FINISH_EMPLOYMENT_STATUS_MESSAGE_LOOP
    if (total == 0 || unemployed == total) {
        
    } else {
        AVERAGE_WAGE = total_wages / (total - unemployed);
    }
    
	return 0; /* Returning zero means the agent is not removed */
}
