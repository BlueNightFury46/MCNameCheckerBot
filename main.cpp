#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/once.h>
#include <dpp/utility.h>
#include <fstream>
#include <dpp/dpp.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iterator>

//Set file path for token
const std::string file_path = "../token.txt";


const std::string MOJANG_API_URL = "https://api.mojang.com/users/profiles/minecraft/";

//File reader function
void get_token(std::string* TOKEN, std::string PATH){
 
   std::fstream file(PATH);

   std::getline(file, *TOKEN);

   file.close();
 
}

size_t write_data(char *buffer, size_t size, size_t nmemb, void *userp) {


      //Append the buffer data to our string passed in with the WRITE_DATA curl option
    ((std::string*)userp)->append(((char*)buffer), (nmemb * size));




    
 return size * nmemb;


}

enum r_value{
  CURL_FAILED = -1,
  USERNAME_NOT_FOUND = false,
  USERNAME_FOUND = true
};

//Get our curl data
int validate_curl_code(std::string username, std::string* f_out);



int main (int argc, char *argv[]) {
 

  std::string token;

  //Call the function to retrieve my bot token
  get_token(&token, file_path);

  //Create the bot
  dpp::cluster bot(token);

  //Set up output
  bot.on_log([&bot](const dpp::log_t& event){

    std::cout << std::endl << ">>" << event.message;

  });

  //Slash command event
  bot.on_slashcommand([&bot](const dpp::slashcommand_t& cmd){

    if(cmd.command.get_command_name() == "validate"){
      std::string param = std::get<std::string>(cmd.get_parameter("username"));
      std::string output;

      int d = validate_curl_code(param, &output);

   
        //If the player isn't found
         if(d == r_value::USERNAME_FOUND){

        dpp::message msg(cmd.command.channel_id, "Player \"" + param + "\" exists");
          bot.log(dpp::loglevel::ll_info, "player exists");
          cmd.reply(msg);
        }

        if(d == r_value::USERNAME_NOT_FOUND){

         //Create message and send it to command issuer
        dpp::message msg(cmd.command.channel_id, "Player \"" + param + "\" does not exist");
        bot.log(dpp::loglevel::ll_info, "no player found");
        cmd.reply(msg);


        }


       if(d == r_value::CURL_FAILED){

        dpp::message msg(cmd.command.channel_id, "FATAL ERROR: CURL FAILED");
          bot.log(dpp::loglevel::ll_info, "curl failed");
        cmd.reply(msg);

        }


      }
    }

  );


  //Register commands once when the bot starts
  bot.on_ready([&bot](const dpp::ready_t& event){

   if(dpp::run_once<struct command_register>()){ 

   //Create the /validate command
   dpp::slashcommand username_validation_cmd("validate", "Checks if the username of a Minecraft player (Java edition) is valid", bot.me.id);
   
   //Add a field to /validate
    dpp::command_option user_valid_opt(dpp::co_string, "username", "The username of the Minecraft player (Java edition)", true);
    //Add the field option to /validate
    username_validation_cmd.add_option(user_valid_opt);
      
    //register /validate
    bot.global_command_create(username_validation_cmd);

}
  });

  //Start the bot
  bot.start(dpp::st_wait);





  return 0;
}



//Our CURL code segment for /validate
int validate_curl_code(std::string username, std::string *f_out){

  CURL* curl = curl_easy_init();

  if(!curl){
    return r_value::CURL_FAILED;
  }
  

  std::string output;

  curl_easy_setopt(curl, CURLOPT_URL, std::string(MOJANG_API_URL + username).c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

  CURLcode code = curl_easy_perform(curl);


  *f_out = output;

  curl_easy_cleanup(curl);



  //Weird solution to find if a name is valid... 
  if(output.find("name") == 50){
    return r_value::USERNAME_FOUND;
  }

  if(output.find("errorMessage") != std::string::npos){
     return r_value::USERNAME_NOT_FOUND;
  }


  //Error checking if CURLE doesn't return CURLE_OK
  if(code != CURLE_OK){
     switch(code){

     
      case CURLE_FAILED_INIT:{
        return r_value::CURL_FAILED;
      }




    }
  }



  return r_value::USERNAME_FOUND;

}











