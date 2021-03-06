#include "../header.h"
#include "../firm_agent_header.h"

/*
 * \fn: int firm_credit_check_interest_rate()
 * \brief: it receives interest rate from the central bank and updates interest to be paid on loans.
 */
int firm_credit_check_interest_rate()
{
    double rcb;
    START_INTEREST_RATE_MESSAGE_LOOP
    /* The 1 percent increase is determined by the model. */
    rcb = interest_rate_message->rate;
    LOANS_INTEREST_RATE = rcb + 0.01;
    
	FINISH_INTEREST_RATE_MESSAGE_LOOP
        
	return 0; /* Returning zero means the agent is not removed */
}


/*
 * \fn: int firm_credit_check_tax_rate()
 * \brief: Recieved from the government.
 */
int firm_credit_check_tax_rate()
{

    START_LABOUR_TAX_RATE_MESSAGE_LOOP
    LABOUR_TAX_RATE = labour_tax_rate_message->value;
	FINISH_LABOUR_TAX_RATE_MESSAGE_LOOP
    
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_compute_income_statement()
 * \brief: Firm computes its income statement.
 */
int firm_credit_compute_income_statement()
{
    int bank;
    double to_be_paid;
    
    
    TOTAL_INTEREST_PAYMENTS = 0;
    
    for (int i = 0; i < 2; i++) {
        bank = LOAN_LIST[i].bank_id;
        to_be_paid = LOAN_LIST[i].amount * (LOANS_INTEREST_RATE / 4);
        LOAN_LIST[i].to_be_paid = to_be_paid;
        TOTAL_INTEREST_PAYMENTS += to_be_paid;
    }

    /* Labour cost is decremented from liquidity on a monthly basis.
     Costs (wages) and revenues (from sales) are updated incrementally.
     */
    
    OPERATING_COSTS = LABOUR_COSTS + TOTAL_INTEREST_PAYMENTS;
    EBIT = REVENUES - (LABOUR_COSTS + RAWMATERIAL_COSTS + CGOODS_COSTS);
    NET_EARNINGS = EBIT - TOTAL_INTEREST_PAYMENTS;

    if (DATA_COLLECTION_MODE && COLLECT_FIRM_DATA) {
        char * filename;
        FILE * file1;
        filename = malloc(100*sizeof(char));
        filename[0]=0;
        if (FIRMTYPE == 1) {
            strcpy(filename, "./outputs/data/Constructor_Firm_Quarterly_IncomeStatement.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %f %f %f %f %f %f\n", IT_NO, ID, REVENUES, OPERATING_COSTS, LABOUR_COSTS, TOTAL_INTEREST_PAYMENTS, EBIT, NET_EARNINGS);
        } else if (FIRMTYPE == 0) {
            strcpy(filename, "./outputs/data/Firm_Quarterly_IncomeStatement.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %f %f %f %f %f %f\n", IT_NO, ID, REVENUES, OPERATING_COSTS, LABOUR_COSTS, TOTAL_INTEREST_PAYMENTS, EBIT, NET_EARNINGS);
        } else {
            strcpy(filename, "./outputs/data/Export_Firm_Quarterly_IncomeStatement.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %f %f %f %f %f %f\n", IT_NO, ID, REVENUES, OPERATING_COSTS, LABOUR_COSTS, TOTAL_INTEREST_PAYMENTS, EBIT, NET_EARNINGS);
        }
        
        /*** Balancesheet Verification.
        file1 = fopen(filename,"a");
        fprintf("%d %d %f %f %f\n",IT_NO, ID, DEBT, LOAN_LIST[0].amount, LOAN_LIST[1].amount);
         */
        
        fclose(file1);
        free(filename);
    }
    
    REVENUES = 0;
    OPERATING_COSTS = 0;
    LABOUR_COSTS = 0;
    RAWMATERIAL_COSTS = 0;
    CGOODS_COSTS = 0;
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_investment_decisions()
 * \brief: 
 */
int firm_credit_investment_decisions()
{
    RETAINED_EARNINGS = 0;
    PLANNED_INVESTMENT_COSTS = 0;
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_compute_dividends()
 * \brief: Within this implementation the firm sends all of net income
 * to the Fund Equity to be distributed equally to households.
 */
int firm_credit_compute_dividends()
{
    DIVIDENDS_TO_BE_PAID = NET_EARNINGS - RETAINED_EARNINGS;
    DIVIDENDS_PAID = 0;
    RETAINED_EARNINGS = 0;
    if (DIVIDENDS_TO_BE_PAID < 0) {
        if (PRINT_DEBUG_MODE) {
            printf("Firm ID = %d, has no positive earnings to send out to share holders. \n", ID);
        }
        DIVIDENDS_TO_BE_PAID = 0;
    }
        
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_check_liquidity_need()
 * \brief: Firm checks its liquidity need. The information will be used for the decisio
 * to enter the credit market or not.
 */
int firm_credit_check_liquidity_need()
{
  
    LIQUIDITY_NEED = DIVIDENDS_TO_BE_PAID + TOTAL_INTEREST_PAYMENTS + PLANNED_INVESTMENT_COSTS - LIQUIDITY;
    
    //printf("At Start of the Quarter: Firm ID = %d, Liquidity: %f, Liquidity Need: %f\n", ID, LIQUIDITY, LIQUIDITY_NEED);
    
    /*The variable is used for statistics as state variables. */
    ISLIQUIDSHORT = 0;
    HASLOAN = 0;
    HASINVESTMENT = 0;
    ISILLIQUID = 0;
    ISINSOLVENT = 0;
    
    if (LIQUIDITY_NEED > 0.1) {
        ISLIQUIDSHORT = 1;
    }
    else{LIQUIDITY_NEED = 0;}
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_demand_loans_1()
 * \brief: Firm goes to its own bank for the loan request.
 */
int firm_credit_demand_loans_1()
{
    if (LIQUIDITY_NEED > 0){
        add_firm_bank_loan_request_1_message(ID, BANK_ID, LIQUIDITY_NEED);
        //printf("Firm Stage 1: Firm ID = %d --> Bank ID = %d R = %f,  Current D = %f \n", ID, BANK_ID, LIQUIDITY_NEED, DEBT);
    }
    
    
    
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_borrow_loans_1()
 * \brief: The function is run when there is liquidity need.
 */
int firm_credit_borrow_loans_1()
{
    double amount = 0;

    START_BANK_FIRM_LOAN_ACKNOWLEDGE_1_MESSAGE_LOOP
    amount = bank_firm_loan_acknowledge_1_message->amount;
    LIQUIDITY += amount;
    DEBT += amount;
    LOAN_LIST[0].amount += amount;
    //printf("Firm ID = %d @ Loan Stage 1 received %f of loans. Updated Debt = %f \n", ID, amount, DEBT);
    FINISH_BANK_FIRM_LOAN_ACKNOWLEDGE_1_MESSAGE_LOOP
    
    
    
    if (PRINT_DEBUG_MODE){
        printf("Firm ID = %d @ Loan Stage 1 received %f of loans. \n", ID, amount);
    }
    
    if (amount >= LIQUIDITY_NEED){
        LIQUIDITY_NEED = 0;
        HASLOAN = 1;
    }
    /* Shall we allow partial loans?
     If so at bank side partial loan should be processed.
     */
    else{
        LIQUIDITY_NEED -= amount;
        add_firm_bank_loan_request_2_message(ID, LOAN_LIST[1].bank_id, LIQUIDITY_NEED);
        //printf("Firm Stage 2: Firm ID = %d --> Bank ID = %d R = %f,  Current D = %f \n", ID, LOAN_LIST[1].bank_id, LIQUIDITY_NEED, DEBT);
    }
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_borrow_loans_2()
 * \brief:
 */
int firm_credit_borrow_loans_2()
{
    double amount = 0;
    
    START_BANK_FIRM_LOAN_ACKNOWLEDGE_2_MESSAGE_LOOP
    amount = bank_firm_loan_acknowledge_2_message->amount;
    LIQUIDITY += amount;
    DEBT += amount;
    LOAN_LIST[1].amount += amount;
    //printf("Firm ID = %d @ Loan Stage 2 received %f of loans. Updated Debt = %f \n", ID, amount, DEBT);
    FINISH_BANK_FIRM_LOAN_ACKNOWLEDGE_2_MESSAGE_LOOP

    
    if (PRINT_DEBUG_MODE){
        printf("Firm ID = %d @ Loan Stage 2 received %f of loans. \n", ID, amount);
    }

    if (amount >= LIQUIDITY_NEED){
        LIQUIDITY_NEED = 0;
        HASLOAN = 1;
    }
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_request_equityfund_investment()
 * \brief: Firm goes to equity fund to get the required amount of liquidity it needs. Dividend payments are 
 * deferred.
 */
int firm_credit_request_equityfund_investment()
{
    /* New liquidity need is computed. */
    LIQUIDITY_NEED = TOTAL_INTEREST_PAYMENTS - LIQUIDITY;
    
    if (LIQUIDITY_NEED <= 0) {
        DIVIDENDS_TO_BE_PAID = 0;
        PLANNED_INVESTMENT_COSTS = 0;
        LIQUIDITY_NEED = 0 ;
    }
    else {
        if (TOTAL_ASSETS == 0) {
            if (DEBT <= 0) {
                add_fund_request_message(ID, LIQUIDITY_NEED);
            }
        return 0;
        }
        
        if ((EQUITY / TOTAL_ASSETS) > FIRMS_MINIMUM_EQUITY_RATIO) {
            add_fund_request_message(ID, LIQUIDITY_NEED);
        }
    }
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_check_equityfund_investment()
 * \brief: Firms check if they are invested by Equity Fund for their liquidity need.
 */
int firm_credit_check_equityfund_investment()
{
    double amount = 0;
    
    START_FUND_REQUEST_ACK_MESSAGE_LOOP
    amount = fund_request_ack_message->amount;
 	FINISH_FUND_REQUEST_ACK_MESSAGE_LOOP
    
    if (amount > 0) {
        LIQUIDITY += amount;
        HASINVESTMENT = 1;
        EQUITY += amount;
        LIQUIDITY_NEED = 0;
        if (PRINT_DEBUG_MODE){
            printf("Firm ID = %d @ Loan - Equity Fund received %f amount of funding investment. \n", ID, amount);
        }
    }
	return 0; /* Returning zero means the agent is not removed */
}


/*
 * \fn: int firm_credit_illiquidity_bankrupt()
 * \brief: Firm debt is restructured.
 */
int firm_credit_illiquidity_bankrupt()
{
    ISILLIQUID = 1;
    if (PRINT_DEBUG_MODE) {
        printf("Firm ID = %d is illiquidity bankrupt!\n", ID);
    }
    
    double new_loans;
    double ratio, current_amount, new_amount, delta_amount;
    int bank;
    
    
    /* Maximum amount of debts that can be paid.
     */
    if (EBIT < 0){
        new_loans = 0;}
    else {
        new_loans = EBIT / (LOANS_INTEREST_RATE / 4);
    }
    
    if (DEBT > 0){
        ratio = new_loans / DEBT;}
    else {
        ratio = 0;
    }
    
    if (ratio >= 1) { return 0;}
    
    DEBT = 0;
    for (int i = 0; i < 2; i++) {
        bank = LOAN_LIST[i].bank_id;
        current_amount = LOAN_LIST[i].amount;
        new_amount = current_amount * ratio;
        LOAN_LIST[i].amount = new_amount;
        DEBT += new_amount;
        delta_amount = current_amount - new_amount;
        
        if (delta_amount > 0.1) {
            add_loan_writeoff_message(bank, delta_amount);
            if (WARNING_MODE) {
                if (delta_amount < 0) {
                    printf("Warning @firm_credit_illiquidity_bankrupt(): The illiquid Firm ID = %d new loan request is higher than its existing loan to Bank ID = %d, The difference is = %f \n", ID, bank, delta_amount);
                }
            }
            
            //printf("Illiquidity: Firm ID = %d, Bank = %d  Writeoff =  %f \n",ID,bank, delta_amount);
            
            if (PRINT_DEBUG_MODE) {
                printf("Firm ID = %d illiquidity bankrupt burden on Bank = %d is = %f \n",ID,bank, delta_amount);
            }
            
            if (DATA_COLLECTION_MODE) {
                char * filename;
                FILE * file1;
                filename = malloc(40*sizeof(char));
                filename[0]=0;
                strcpy(filename, "./outputs/data/BankruptcyInspection.txt");
                file1 = fopen(filename,"a");
                fprintf(file1,"%d %d %s %s %d %f\n",IT_NO, ID, "Firm", "Illiquidity", bank, delta_amount);
                fclose(file1);
                free(filename);
            }
        }
    }
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_pay_interest_on_loans()
 * \brief:
 */
int firm_credit_pay_interest_on_loans()
{

    int bank;
    double to_be_paid;
    
    for (int i = 0; i < 2; i++) {
        bank = LOAN_LIST[i].bank_id;
        to_be_paid = LOAN_LIST[i].to_be_paid;
        LIQUIDITY -= to_be_paid;
        add_firm_bank_interest_on_loan_message(bank, to_be_paid);
    }

 	return 0; /* Returning zero means the agent is not removed */
}


/*
 * \fn: int firm_credit_pay_dividends()
 * \brief: Firm send dividends to to the Fund Equity to be distributed
 * to households.
 */
int firm_credit_pay_dividends()
{
    DIVIDENDS_PAID = DIVIDENDS_TO_BE_PAID;
    
    if (DIVIDENDS_PAID > 0) {
        add_firm_net_profit_message(ID, FIRMTYPE, DIVIDENDS_PAID);
        LIQUIDITY -= DIVIDENDS_PAID;
    }
    
    if (DATA_COLLECTION_MODE && COLLECT_FIRM_DATA) {
        char * filename;
        FILE * file1;
        filename = malloc(100*sizeof(char));
        filename[0]=0;
        if (FIRMTYPE == 1) {
            strcpy(filename, "./outputs/data/Constructor_Firm_Quarterly_Dividends.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %f %f\n", IT_NO, ID, DIVIDENDS_PAID, DIVIDENDS_TO_BE_PAID);
        } else if (FIRMTYPE == 0) {
            strcpy(filename, "./outputs/data/Firm_Quarterly_Dividends.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %f %f\n", IT_NO, ID, DIVIDENDS_PAID, DIVIDENDS_TO_BE_PAID);
        } else {
            strcpy(filename, "./outputs/data/Export_Firm_Quarterly_Dividends.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %f %f\n", IT_NO, ID, DIVIDENDS_PAID, DIVIDENDS_TO_BE_PAID);
        }
        fclose(file1);
        free(filename);
    }
    
    DIVIDENDS_TO_BE_PAID = 0;
    
	return 0; /* Returning zero means the agent is not removed */
}


/*
 * \fn: int firm_credit_do_balance_sheet()
 * \brief: Firm does the balance sheet accounting.
 */
int firm_credit_do_balance_sheet()
{
    
    if (FIRMTYPE == 1) {
        TOTAL_ASSETS  = INVENTORY * UNIT_HOUSE_PRICE;
    } else if (FIRMTYPE == 0) {
        TOTAL_ASSETS = INVENTORY * UNIT_GOODS_PRICE;
    } else {
        TOTAL_ASSETS = 0;
    }
    /* Capital goods are valued using the initial Exchange Rate */
    TOTAL_ASSETS += LIQUIDITY + CAPITAL_GOODS * UNIT_CGOODS_PRICE * EXCHANGE_RATE_REFERENCE;
    EQUITY = TOTAL_ASSETS - DEBT;
    
    if (EQUITY < 0) {
        ISINSOLVENT = 1; }
    
    if (DATA_COLLECTION_MODE && COLLECT_FIRM_DATA) {
        
        char * filename;
        FILE * file1;
        filename = malloc(100*sizeof(char));
        filename[0]=0;
        if (FIRMTYPE == 1) {
            strcpy(filename, "./outputs/data/Constructor_Firm_Quarterly_BalanceSheet.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %d %d %d %f %d %d %f %f %d %f %f %d %f %f\n",IT_NO, ID, ISLIQUIDSHORT, HASLOAN, HASINVESTMENT, LIQUIDITY_NEED, ISINSOLVENT, ISILLIQUID, TOTAL_ASSETS, LIQUIDITY, INVENTORY, UNIT_HOUSE_PRICE, UNIT_CGOODS_PRICE * EXCHANGE_RATE, CAPITAL_GOODS, DEBT, EQUITY);
        } else if (FIRMTYPE == 0) {
            strcpy(filename, "./outputs/data/Firm_Quarterly_BalanceSheet.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %d %d %d %f %d %d %f %f %d %f %f %d %f %f\n",IT_NO, ID, ISLIQUIDSHORT, HASLOAN, HASINVESTMENT, LIQUIDITY_NEED, ISINSOLVENT, ISILLIQUID, TOTAL_ASSETS, LIQUIDITY, INVENTORY, UNIT_GOODS_PRICE, UNIT_CGOODS_PRICE * EXCHANGE_RATE, CAPITAL_GOODS, DEBT, EQUITY);
        } else {
            strcpy(filename, "./outputs/data/Export_Firm_Quarterly_BalanceSheet.txt");
            file1 = fopen(filename,"a");
            fprintf(file1,"%d %d %d %d %d %f %d %d %f %f %d %f %f %d %f %f\n",IT_NO, ID, ISLIQUIDSHORT, HASLOAN, HASINVESTMENT, LIQUIDITY_NEED, ISINSOLVENT, ISILLIQUID, TOTAL_ASSETS, LIQUIDITY, INVENTORY, UNIT_XGOODS_PRICE, UNIT_CGOODS_PRICE * EXCHANGE_RATE, CAPITAL_GOODS, DEBT, EQUITY);
        }
        fclose(file1);
        free(filename);
    }
    
	return 0; /* Returning zero means the agent is not removed */
}

/*
 * \fn: int firm_credit_insolvency_bankruptcy()
 * \brief:
 */
int firm_credit_insolvency_bankruptcy()
{
    
    /* The firm will skip production for the first time and enters the labour market.
     The variable below is used for that purpose in production and labour markets.
     */
    
    /* Writeoff debts.
     */
    int bank;
    double amount;
    
    
    //printf("Firm ID = %d is insolvent bankrupt!! Amount = %f \n", ID, DEBT);
    
    if (PRINT_DEBUG_MODE) {
        printf("Firm ID = %d is insolvent bankrupt!! \n", ID);
    }
    
    
    add_firm_bank_insolvent_account_message(BANK_ID, LIQUIDITY);
    LIQUIDITY = 0;
    DEBT = 0;
    REVENUES = 0;
    OPERATING_COSTS = 0;
    LABOUR_COSTS = 0;
    TOTAL_INTEREST_PAYMENTS = 0;
    RAWMATERIAL_COSTS = 0;
    CGOODS_COSTS = 0;
    /* Physical capital etc are kept the same.
     */
    
    
    if (FIRMTYPE == 0) {
        INVENTORY = LABOUR_PRODUCTIVITY * 1;
        TOTAL_ASSETS = INVENTORY * AVERAGE_GOODS_PRICE + LIQUIDITY;
        UNIT_GOODS_PRICE = AVERAGE_GOODS_PRICE;
        SALES = 0;
        /* UNIT_COST is inherited. */
    } else if (FIRMTYPE == 1) {
        INVENTORY = 0;
        TOTAL_ASSETS = CAPITAL_PRODUCTIVITY * 1 + LIQUIDITY;
        SALES = 0;
        /* Constructor firms keep the averega house prices, current projects, etc */
    } else {
        INVENTORY = 0;
        TOTAL_ASSETS = LIQUIDITY;
        /* Export firms only inherit the capital goods and previous quota (SALES)*/
    }
    
    for (int i = 0; i < 2; i++) {
        bank = LOAN_LIST[i].bank_id;
        amount = LOAN_LIST[i].amount;
        if (amount > 0.001) {
            add_loan_writeoff_message(bank, amount);
            
            //printf("Insolvency: Firm ID = %d, Bank = %d  Writeoff =  %f, Updated  \n",ID,bank, amount);
            
            if (DATA_COLLECTION_MODE) {
                char * filename;
                FILE * file1;
                filename = malloc(40*sizeof(char));
                filename[0]=0;
                strcpy(filename, "./outputs/data/BankruptcyInspection.txt");
                file1 = fopen(filename,"a");
                fprintf(file1,"%d %d %s %s %d %f\n",IT_NO, ID, "Firm", "Insolvency", bank, amount);
                fclose(file1);
                free(filename);
            }
        }
        LOAN_LIST[i].amount = 0;
    }
    
    
    /* Getting initial loan */
    TOTAL_ASSETS += CAPITAL_GOODS * UNIT_CGOODS_PRICE * EXCHANGE_RATE_REFERENCE;
    //DEBT = TOTAL_ASSETS / (1 + FIRM_STARTUP_LEVERAGE);
    /* There is one single employee at the firm after becoming insolvent */
    
    DEBT = (1 * WAGE_OFFER * 0.2) / LOANS_INTEREST_RATE;
    LOAN_LIST[0].amount = DEBT;
    add_new_entrant_loan_message(ID, BANK_ID, DEBT);
    EQUITY = TOTAL_ASSETS - DEBT;
    
	return 0; /* Returning zero means the agent is not removed */
}

