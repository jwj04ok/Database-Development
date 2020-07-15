
#ifndef FolderView_h
#define FolderView_h

#include "FolderReader.hpp"
#include "View.hpp"

namespace ECE141 {
	class FolderView :public View, public FolderListener {
	public:
		FolderView(const FolderReader& aReader, const char* anExtension = "db")
			:reader(aReader), extension(anExtension), stream(nullptr) {}

		bool operator()(const std::string& aString) {
			(*stream) << aString << std::endl;
			return true;
		}

		virtual bool show(std::ostream& aStream) {
			stream = &aStream;
			aStream << "Showing databases:" << std::endl;
			reader.each(*this, "db");
			return true;
		}

		const FolderReader& reader;
		const char*			extension;
		std::ostream*		stream;

	};
}





#endif /* FolderView_h */