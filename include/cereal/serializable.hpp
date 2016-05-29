/*! \file serializable.hpp
    \brief Macro that generates serializable member variables*/
/*
/*
  Copyright (c) 2014, David Jensen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of cereal nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL RANDOLPH VOORHIES OR SHANE GRANT BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CEREAL_SERIALIZABLE_HPP_
#define CEREAL_SERIALIZABLE_HPP_

#include <boost/preprocessor.hpp>

#define REM(...) __VA_ARGS__
#define EAT(...)

// Strip off the type
#define STRIP(x) EAT x
// Show the type without parenthesis
#define PAIR(x) REM x

/* The following zipping utility macros are copied from Evgeny Panasyuk stackoverflow answer at:
   http://stackoverflow.com/questions/26475453/how-to-use-boostpreprocessor-to-unzip-a-sequence */

// (Original stackoverflow comment) Such technique is used at:
//   http://www.boost.org/doc/libs/1_56_0/boost/fusion/adapted/struct/define_struct.hpp
#define AUXILIARY_0(...) ((__VA_ARGS__)) AUXILIARY_1
#define AUXILIARY_1(...) ((__VA_ARGS__)) AUXILIARY_0
#define AUXILIARY_0_END
#define AUXILIARY_1_END

#define REMOVE_PARENTHESES(...) __VA_ARGS__

#define COMMA_SEPARATED(r, data, i, elem) \
    BOOST_PP_COMMA_IF(i) BOOST_PP_VARIADIC_ELEM(data, REMOVE_PARENTHESES elem)

#define ZIPPED_TO_SEQ(zipped) \
    BOOST_PP_SEQ_POP_FRONT(BOOST_PP_CAT(AUXILIARY_0(0)zipped,_END))

#define FOR_EACH_ZIPPED_I(macro, zipped) \
    BOOST_PP_SEQ_FOR_EACH_I(macro, 0, ZIPPED_TO_SEQ(zipped))

// UNZIP is used to join a boost sequence (x)(y)(z) to a comma separated list x,y,z
#define UNZIP(zipped) \
 FOR_EACH_ZIPPED_I(COMMA_SEPARATED, zipped)

// End of zipping utilities

// Takes MEMBERDECLLIST z item and and returns only its name
#define REMOVE_TYPEINFO(x, data, z) (STRIP(z))

// Creates a member variable declaration from a MEMBERDECLLIST item x
#define DECLARE_EACH(r, data, i, x) PAIR(x);

/* Creates a name value pair, we can unfortunately not use the MAKE_NVP macro because we need BOOST_STRINGIZE when being
   inside BOOST_PP_SEQ_FOR_EACH */
#define NVP_EACH(r, data, x) (::cereal::make_nvp(BOOST_STRINGIZE(x), x))

// Takes a list of member variable declarations for example: (int) x, (bool) y and returns the names x,y
#define SELECT_NAMES_FROM_MEMBERDECLLIST(...)  \
    UNZIP(BOOST_PP_SEQ_FOR_EACH(REMOVE_TYPEINFO,data,BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

// Takes a list of member variable declarations for example: (int) x, (bool) y
// and turns each name into a name value pair -> {"x",x},{"y",y}
#define MAKE_NVP_FOR_ALL_NAMES_IN_MEMBERDECLLIST(...) \
    BOOST_PP_SEQ_FOR_EACH(NVP_EACH,data,BOOST_PP_VARIADIC_TO_SEQ(SELECT_NAMES_FROM_MEMBERDECLLIST(__VA_ARGS__)))

// Creates declarations out of a declaration list
#define DECLARE_ALL_IN_MEMBERDECLLIST(...) \
    BOOST_PP_SEQ_FOR_EACH_I(DECLARE_EACH, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))


// ######################################################################
//! Declares and adds all items in a member variable declaration list to a serialize function
/*! The arguments should provide a type in parenthesis along with a name.

    -- Example --

    struct Car{
        SERIALIZABLE
        (
            (std::string) name,
            (int) age,
            (bool) isBest
        )
    };

    -- Would expand to --

    struct Car{
        std::string name;
        int age;
        bool isBest;

        template <class Archive>
        void serialize( Archive & ar ) const {
          ar( ::cereal::make_nvp("name",name),
              ::cereal::make_nvp("age",age),
              ::cereal::make_nvp("isBest",isBest));
        }
    };

 */
#define SERIALIZABLE(...) \
DECLARE_ALL_IN_MEMBERDECLLIST(__VA_ARGS__) \
template <class Archive> \
void serialize( Archive & ar ) const { \
  ar( UNZIP(MAKE_NVP_FOR_ALL_NAMES_IN_MEMBERDECLLIST(__VA_ARGS__))); \
}

#endif //CEREAL_SERIALIZABLE_HPP_
