#pragma once

#include <city.h>

#include <stats/UniquesHashSet.h>

#include <DB/IO/WriteHelpers.h>
#include <DB/IO/ReadHelpers.h>
#include <DB/IO/WriteBufferFromString.h>

#include <DB/DataTypes/DataTypesNumberFixed.h>
#include <DB/DataTypes/DataTypeString.h>

#include <DB/AggregateFunctions/IUnaryAggregateFunction.h>


namespace DB
{


template <typename T> struct AggregateFunctionUniqTraits;

template <> struct AggregateFunctionUniqTraits<UInt64>
{
	static UInt64 hash(UInt64 x) { return x; }
};

template <> struct AggregateFunctionUniqTraits<Int64>
{
	static UInt64 hash(Int64 x) { return x; }
};

template <> struct AggregateFunctionUniqTraits<Float64>
{
	static UInt64 hash(Float64 x)
	{
		UInt64 res = 0;
		memcpy(reinterpret_cast<char *>(&res), reinterpret_cast<char *>(&x), sizeof(x));
		return res;
	}
};

template <> struct AggregateFunctionUniqTraits<String>
{
	/// Имейте ввиду, что вычисление приближённое.
	static UInt64 hash(const String & x) { return CityHash64(x.data(), x.size()); }
};


struct AggregateFunctionUniqData
{
	UniquesHashSet set;
};


/// Приближённо вычисляет количество различных значений.
template <typename T>
class AggregateFunctionUniq : public IUnaryAggregateFunction<AggregateFunctionUniqData>
{
public:
	AggregateFunctionUniq() {}

	String getName() const { return "uniq"; }
	String getTypeID() const { return "uniq_" + TypeName<T>::get(); }

	DataTypePtr getReturnType() const
	{
		return new DataTypeUInt64;
	}

	void setArgument(const DataTypePtr & argument)
	{
	}


	void addOne(AggregateDataPtr place, const Field & value) const
	{
		data(place).set.insert(AggregateFunctionUniqTraits<T>::hash(get<const T &>(value)));
	}

	void merge(AggregateDataPtr place, ConstAggregateDataPtr rhs) const
	{
		data(place).set.merge(data(rhs).set);
	}

	void serialize(ConstAggregateDataPtr place, WriteBuffer & buf) const
	{
		data(place).set.write(buf);
	}

	void deserializeMerge(AggregateDataPtr place, ReadBuffer & buf) const
	{
		UniquesHashSet tmp_set;
		tmp_set.read(buf);
		data(place).set.merge(tmp_set);
	}

	Field getResult(ConstAggregateDataPtr place) const
	{
		return data(place).set.size();
	}
};


/** То же самое, но выводит состояние вычислений в строке в текстовом виде.
  * Используется, если какой-то внешней программе (сейчас это ███████████)
  *  надо получить это состояние и потом использовать по-своему.
  */
template <typename T>
class AggregateFunctionUniqState : public AggregateFunctionUniq<T>
{
public:
	String getName() const { return "uniqState"; }
	String getTypeID() const { return "uniqState_" + TypeName<T>::get(); }

	DataTypePtr getReturnType() const
	{
		return new DataTypeString;
	}

	Field getResult(ConstAggregateDataPtr place) const
	{
		Field res = String();
		WriteBufferFromString wb(get<String &>(res));
		this->data(place).set.writeText(wb);
		return res;
	}
};


/** Принимает два аргумента - значение и условие.
  * Приближённо считает количество различных значений, когда выполнено это условие.
  */
template <typename T>
class AggregateFunctionUniqIf : public IAggregateFunctionHelper<AggregateFunctionUniqData>
{
public:
	AggregateFunctionUniqIf() {}

	String getName() const { return "uniqIf"; }
	String getTypeID() const { return "uniqIf_" + TypeName<T>::get(); }

	DataTypePtr getReturnType() const
	{
		return new DataTypeUInt64;
	}

	void setArguments(const DataTypes & arguments)
	{
		if (!dynamic_cast<const DataTypeUInt8 *>(&*arguments[1]))
			throw Exception("Incorrect type " + arguments[1]->getName() + " of second argument for aggregate function " + getName() + ". Must be UInt8.",
				ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);
	}

	void add(AggregateDataPtr place, const Row & row) const
	{
		if (get<UInt64>(row[1]))
			data(place).set.insert(AggregateFunctionUniqTraits<T>::hash(get<const T &>(row[0])));
	}

	void merge(AggregateDataPtr place, ConstAggregateDataPtr rhs) const
	{
		data(place).set.merge(data(rhs).set);
	}

	void serialize(ConstAggregateDataPtr place, WriteBuffer & buf) const
	{
		data(place).set.write(buf);
	}

	void deserializeMerge(AggregateDataPtr place, ReadBuffer & buf) const
	{
		UniquesHashSet tmp_set;
		tmp_set.read(buf);
		data(place).set.merge(tmp_set);
	}

	Field getResult(ConstAggregateDataPtr place) const
	{
		return data(place).set.size();
	}
};

}
