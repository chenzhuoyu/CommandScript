#include <iostream>
#include "Parser.h"
#include "Tokenizer.h"
#include "SyntaxError.h"

int main()
{
    CommandScript::Compiler::Parser ps(std::make_shared<CommandScript::Compiler::Tokenizer>(R"source(

import file
import logging

Command.setHandler((name, argv, message, request) ->
{
    req = {
        stRequest       -> (0, request),
        sUserQuestion   -> (1, message.split('#')[0]),
        sProposedAnswer -> (2, message.split('#')[1]),
    };

    resp = {
        eCode    -> (0, 0),
        sMessage -> (1, ''),
    }

    result = Command.RPC('SmartAssistant.DobbyChatServer.DobbyChatServantObj').submitChatTraining(req, resp)

    if (result == 0)
        logging.debug('result is %d, %s' % (result, repr(resp)))
    else
        logging.error('result is %d, %s' % (result, repr(resp)))

    return {
        code -> result,
        message -> resp.sMessage,
    }
})

    )source"));

//    try
//    {
        std::cout << ps.parse()->toString() << std::endl;
//    } catch (const CommandScript::Exception::SyntaxError &e)
//    {
//        std::cerr << "row(" << e.row() << "), col(" << e.col() << "): " << e.message() << std::endl;
//    }

    return 0;
}
