#pragma once

#include "BasicTraits.h"
#include "BasicUtils.h"

// ============================================================================
// Convert-zentriertes Framework (Kernbaustein)
// ============================================================================

// Konzept zur Prüfung, ob eine Konvertierung mit convert<T,U> möglich ist
/*
template<typename From, typename To, typename = void>
constexpr bool is_convertible_v = false;

template<typename From, typename To>
constexpr bool is_convertible_v<From, To, std::void_t<decltype(convert<To>(std::declval<From>()))>> = true;

template<typename From, typename To>
concept ConvertibleTo = is_convertible_v<From, To>;
*/

// ============================================================================
// Return-Schutz durch Konvertierbarkeit zum internen CORBA Typ
// ============================================================================

template<typename CorbaTy, typename CppValue, typename = void>
constexpr bool is_returnable_corba_v = false;

template<typename CorbaTy, typename CppValue>
constexpr bool is_returnable_corba_v<
   CorbaTy,
   CppValue,
   std::void_t<
   std::conditional_t<
   CorbaOptionalStruct<CorbaTy>,
   decltype(convert<typename CorbaOptionalTraits<CorbaTy>::value_type>(std::declval<CppValue>())),
   decltype(convert<typename CorbaValueTraits<CorbaTy>::value_type>(std::declval<CppValue>()))
   >
   >
> = true;

template<typename CorbaTy, typename CppValue>
concept ConvertibleToCorba = is_returnable_corba_v<CorbaTy, CppValue>;

// ============================================================================
// CorbaValueWrapper: einheitlicher Wrapper für C++ und CORBA Werte
// ============================================================================

template<typename cpp_ty>
class CorbaValueWrapper {
private:
   using base_type = std::remove_cvref_t<cpp_ty>;
   using storage_type = std::conditional_t<is_std_optional_v<base_type>, base_type, std::optional<base_type>>;
   storage_type theValue;

public:
   using value_type = typename storage_type::value_type;

   // -------------------------------------------------------------------------
   // Konstruktor für C++ Typen via convert
   // -------------------------------------------------------------------------
   template<typename U>
      requires ConvertibleTo<U, value_type> && (!CorbaOptionalStruct<std::remove_cvref_t<U>> &&
                  !CorbaValueStruct<std::remove_cvref_t<U>>)
      explicit CorbaValueWrapper(U&& val) {
      if constexpr (is_std_optional_v<U>) {
         if (val.has_value())
            theValue = convert<value_type>(*val);
         else
            theValue.reset();
      }
      else {
         theValue = convert<value_type>(std::forward<U>(val));
      }
   }

   // -------------------------------------------------------------------------
   // Konstruktor für CORBA Optional-Typen (aus value_type ableitbar)
   // -------------------------------------------------------------------------
   template<CorbaOptionalStruct CorbaOpt>
   explicit CorbaValueWrapper(CorbaOpt const& corba) {
      if (CorbaOptionalTraits<CorbaOpt>::HasValue(corba))
         theValue = CorbaOptionalTraits<CorbaOpt>::GetValue(corba);
      else
         theValue.reset();
   }

   // -------------------------------------------------------------------------
   // Konstruktor für CORBA Value-Typen
   // -------------------------------------------------------------------------
   template<CorbaValueStruct CorbaVal>
   explicit CorbaValueWrapper(CorbaVal const& corba)
      : theValue(CorbaValueTraits<CorbaVal>::GetValue(corba)) {
   }

   // -------------------------------------------------------------------------
   // Zugriff auf C++ Wert
   // -------------------------------------------------------------------------
   value_type const& value() const {
      if constexpr (is_std_optional_v<storage_type>) {
         return *theValue;
      }
      else {
         return theValue;
      }
   }

   bool has_value() const {
      if constexpr (is_std_optional_v<storage_type>) {
         return theValue.has_value();
      }
      else {
         return true;
      }
   }

   // -------------------------------------------------------------------------
   // Rückgabe eines neuen CORBA Rückgabewerts
   // -------------------------------------------------------------------------

   template<typename, typename = void>
   struct HasCorbaReturnTrait : std::false_type {};

   template<typename ty>
   struct HasCorbaReturnTrait<ty, std::void_t<typename CorbaAccessor<ty>::value_type>> : std::true_type {};

   //template<typename ty>
   //constexpr bool HasCorbaReturnTrait_v = HasCorbaReturnTrait<ty>::value;

   template<typename CorbaTy>
      requires HasCorbaReturnTrait<CorbaTy>::value && ConvertibleToCorba<CorbaTy, value_type>
   CorbaTy Return() const {
      CorbaTy result{};
      if constexpr (CorbaOptionalStruct<CorbaTy>) {
         if (has_value())
            CorbaOptionalTraits<CorbaTy>::SetValue(result, value());
         else
            CorbaOptionalTraits<CorbaTy>::Reset(result);
         }
      else {
         CorbaValueTraits<CorbaTy>::SetValue(result, value());
         }
      return result;
      }

   // -------------------------------------------------------------------------
   // Optional: Manuelles Zurücksetzen
   // -------------------------------------------------------------------------
   void Reset() {
      theValue.reset();
   }
};