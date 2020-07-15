#pragma once
#ifndef TableView_hpp
#define TableView_hpp

#include "View.hpp"
#include "Schema.hpp"
#include "Row.hpp"
#include <iomanip>

namespace ECE141 {

	struct TableWriter {
		std::ostream& stream;
		size_t		  width;

		TableWriter(std::ostream& aStream, size_t aWidth) : stream{ aStream }, width(aWidth){}

		template <typename T>
		void write(const T& aValue) {
			stream << std::left << std::setw(int(width)) << aValue << " |";
		}

		void operator()(bool const& aValue) {
			write(aValue);
		}

		void operator()(uint32_t const& aValue) {
			write(aValue);
		}

		void operator()(double const& aValue) {
			write(aValue);
		}

		void operator()(std::string const& aValue) {
			write(aValue);
		}

	};

	class TableView :public View {
	public:
		TableView(Schema& aSchema, RowCollection& aCollection)
			:seperator("+"), rows(aCollection), schema(aSchema) {}

		size_t getWidth(DataType aType) {
			switch (aType) {
			case DataType::bool_type:
			case DataType::int_type:
			case DataType::float_type:   return 9;
			case DataType::datetime_type:
			case DataType::varchar_type: return 20;
			default: break;
			}
			return 10;
		}

		std::string getSeparator() {
			std::string theResult("+");
			for (auto& theAttr : schema.getAttributes()) {
				std::string theSegment(getWidth(theAttr.getType()) + 2, '-');
				theResult += theSegment;
				theResult += "+";
			}
			return theResult;
		}

		TableView& showHeader(const std::string& aSeperator, std::ostream& anOutput) {
			anOutput << aSeperator << "\n";

			std::stringstream theHeader;
			theHeader << "| ";
			for (auto& theAttr : schema.getAttributes()) {
				size_t theLen = theAttr.getName().size();
				int theWidth = (int)(2 + getWidth(theAttr.getType()) - theLen) / 2;
				theHeader << std::setfill(' ') << std::setw(theWidth + 2) <<theAttr.getName()<<std::setw(theWidth+2)<< " |";
			}
			anOutput << theHeader.str() << "\n";
			return *this;
		}

		TableView& showRow(Row& aRow, std::string& aSeparator, std::ostream& anOutput) {
			anOutput << aSeparator << "\n";
			anOutput << "| ";

			const KeyValues& theData = aRow.getColumns();

			for (auto& theAttr : schema.getAttributes()) {
				for (auto& theKV : theData) {
					if (theKV.first == theAttr.getName()) {
						DataType theType = theAttr.getType();
						std::visit(TableWriter{ anOutput,getWidth(theType) }, theKV.second);
					}
				}
			}
			anOutput << "\n";
			return *this;
		}

		TableView& showFooter(const std::string& aSeparator, std::ostream& anOutput) {
			anOutput << aSeparator << "\n";
			return *this;
		}

		std::string getFormattedValue(ValueType& aValue, int aWidth) {
			char theFormat[32];
			char theBuffer[128];
			
			std::sprintf(theFormat, "%s-%ds |", "%", aWidth);
			std::sprintf(theBuffer, theFormat, "hello");
			std::string theResult(theBuffer);
			return theResult;
		}

        bool show(std::ostream& anOutput) {
			std::string theSeperator = getSeparator();

			showHeader(theSeperator, anOutput);
			for (auto& theRow : rows.getRows()) {
				showRow(*theRow, theSeperator, anOutput);
			}
			showFooter(theSeperator, anOutput);
			anOutput << rows.getRows().size() << " rows in set" <<std::endl;
			return true;
		}



	protected:
		Schema			  &schema;
		RowCollection	  &rows;
		std::vector<int>  width;
		std::string	      seperator;
	};
	
}

#endif /* TableView_hpp */
