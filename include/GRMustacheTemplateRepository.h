// The MIT License
// 
// Copyright (c) 2012 Gwendal Roué
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#import <Foundation/Foundation.h>
#import "GRMustacheAvailabilityMacros.h"
#import "GRMustache.h"

@class GRMustacheTemplate;
@class GRMustacheTemplateRepository;

/**
 The protocol for a GRMustacheTemplateRepository's dataSource.
 
 The dataSource's responsability is to provide Mustache template strings for template and partial names.
 
 @see GRMustacheTemplateRepository
 */
@protocol GRMustacheTemplateRepositoryDataSource <NSObject>
@required

/**
 Returns a template ID, that is to say an object that uniquely identifies a template or a template partial.
 
 The class of this ID is opaque: your implementation of a GRMustacheTemplateRepositoryDataSource
 would define, for itself, what kind of object would identity a template or a partial.
 
 For instance, a file-based data source may use NSString objects containing paths to the templates.
 
 You should try to choose "human-readable" template IDs. That is because template IDs are embedded in
 the description of errors that may happen during a template processing, in order to help the library
 user locate, and fix, the faulting template.
 
 Whenever relevant, template and partial hierarchies are supported via the _baseTemplateID_ parameter: it contains
 the template ID of the enclosing template, or nil when the data source is asked for a template ID for a partial
 that is referred from a raw template string (see [GRMustacheTemplateRepository templateFromString:error:]).
 
 Not all data sources have to implement hierarchies: they can simply ignore this parameter.
 
 The returned value can be nil: the library user would then eventually get an NSError of domain
 GRMustacheErrorDomain and code GRMustacheErrorCodeTemplateNotFound.
 
 @return a template ID
 @param templateRepository The GRMustacheTemplateRepository asking for a template ID.
 @param name The name of the template or template partial.
 @param baseTemplateID The template ID of the enclosing template, or nil.
 */
- (id<NSCopying>)templateRepository:(GRMustacheTemplateRepository *)templateRepository templateIDForName:(NSString *)name relativeToTemplateID:(id)baseTemplateID AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Provided with a template ID that comes from templateRepository:templateIDForName:relativeToTemplateID:,
 returns a Mustache template string.

 For instance, a file-based data source may interpret the template ID as a NSString object
 containing paths to the template, and return the file content.
 
 A few words about the way your implementation of this method must handle errors:
 
 As usually, whenever this method returns nil, the _outError_ parameter should point to
 a valid NSError. This NSError would eventually reach the library user.
 
 However, should you "forget" to set the _outError_ parameter, GRMustache would generate for you
 an NSError of domain GRMustacheErrorDomain and code GRMustacheErrorCodeTemplateNotFound.

 @return a Mustache template string
 @param templateRepository The GRMustacheTemplateRepository asking for a Mustache template string.
 @param templateID The template ID of the template
 @param outError If there is an error returning a template string, upon return contains nil, or an NSError object that describes the problem.
 */
- (NSString *)templateRepository:(GRMustacheTemplateRepository *)templateRepository templateStringForTemplateID:(id)templateID error:(NSError **)outError AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;
@end


/**
 Given a data source that provides Mustache template strings, a GRMustacheTemplateRepository's responsability
 is to provide GRMustacheTemplate instances.
 
 You may provide your own template string data source. However common cases such as loading templates
 from URLs, files, bundle resources, and dictionaries, are already implemented.
 
 @see GRMustacheTemplate
 @see GRMustacheTemplateRepositoryDataSource
 */
@interface GRMustacheTemplateRepository : NSObject {
@private
    id<GRMustacheTemplateRepositoryDataSource> _dataSource;
    NSMutableDictionary *_templateForTemplateID;
    id _currentlyParsedTemplateID;
}

/**
 The repository's data source.
 */
@property (nonatomic, assign) id<GRMustacheTemplateRepositoryDataSource> dataSource AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

#if !TARGET_OS_IPHONE || __IPHONE_OS_VERSION_MAX_ALLOWED >= 40000

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from files of extension .mustache,
 encoded in UTF8, stored in the provided base URL.
 
 For instance:
 
 @code
 // Creates a repository for templates stored in /path/to/templates
 NSURL *baseURL = [NSURL fileURLWithPath:@"/path/to/templates"];
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithBaseURL:baseURL];
 
 // Returns a template for the file stored in /path/to/templates/profile.mustache
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 A partial tag `{{>partial}}` loads a partial template stored in a file named `partial.mustache`, located in the enclosing template's directory.
 
 You may use the slash `/`, and `..`, in order to navigate the URL hierarchical system: `{{>partials/achievements}}`
 would load /path/to/templates/partials/achievements.mustache, if invoked from /path/to/templates/profile.mustache.
 
 When you ask the repository to parse a raw template string, partials are loaded from the base URL:
 
 @code
 // The partial would be loaded from /path/to/templates/partials/achievements.mustache
 GRMustacheTemplate *template = [repository templateFromString:@"{{>partials/achievements}}" error:NULL];
 @endcode

 @return a GRMustacheTemplateRepository
 @param URL the base URL where to look templates from.
 */
+ (id)templateRepositoryWithBaseURL:(NSURL *)URL AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from files of provided extension,
 encoded in UTF8, stored in the provided base URL.
 
 For instance:
 
 @code
 // Creates a repository for templates of extension `.txt` stored in /path/to/templates
 NSURL *baseURL = [NSURL fileURLWithPath:@"/path/to/templates"];
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithBaseURL:baseURL
                                                                                      templateExtension:@"txt"];
 
 // Returns a template for the file stored in /path/to/templates/profile.txt
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 A partial tag `{{>partial}}` loads a partial template stored in a file named `partial.txt`, located in the enclosing template's directory.
 
 You may use the slash `/`, and `..`, in order to navigate the URL hierarchical system: `{{>partials/achievements}}`
 would load /path/to/templates/partials/achievements.txt, if invoked from /path/to/templates/profile.txt.
 
 When you ask the repository to parse a raw template string, partials are loaded from the base URL:
 
 @code
 // The partial would be loaded from /path/to/templates/partials/achievements.txt
 GRMustacheTemplate *template = [repository templateFromString:@"{{>partials/achievements}}" error:NULL];
 @endcode
 
 @return a GRMustacheTemplateRepository
 @param URL The base URL where to look templates from.
 @param ext The extension of template files.
 */
+ (id)templateRepositoryWithBaseURL:(NSURL *)URL templateExtension:(NSString *)ext AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from files of provided extension,
 encoded in the provided encoding, stored in the provided base URL.
 
 For instance:
 
 @code
 // Creates a repository for templates of extension `.txt` stored in /path/to/templates, encoded with NSMacOSRomanStringEncoding:
 NSURL *baseURL = [NSURL fileURLWithPath:@"/path/to/templates"];
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithBaseURL:baseURL
                                                                                      templateExtension:@"txt"
                                                                                               encoding:NSMacOSRomanStringEncoding];
 
 // Returns a template for the file stored in /path/to/templates/profile.txt
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 A partial tag `{{>partial}}` loads a partial template stored in a file named `partial.txt`, located in the enclosing template's directory.
 
 You may use the slash `/`, and `..`, in order to navigate the URL hierarchical system: `{{>partials/achievements}}`
 would load /path/to/templates/partials/achievements.txt, if invoked from /path/to/templates/profile.txt.
 
 When you ask the repository to parse a raw template string, partials are loaded from the base URL:
 
 @code
 // The partial would be loaded from /path/to/templates/partials/achievements.txt
 GRMustacheTemplate *template = [repository templateFromString:@"{{>partials/achievements}}" error:NULL];
 @endcode
 
 @return a GRMustacheTemplateRepository
 @param URL The base URL where to look templates from.
 @param ext The extension of template files.
 @param encoding The encoding of template files.
 */
+ (id)templateRepositoryWithBaseURL:(NSURL *)URL templateExtension:(NSString *)ext encoding:(NSStringEncoding)encoding AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;
#endif /* if !TARGET_OS_IPHONE || __IPHONE_OS_VERSION_MAX_ALLOWED >= 40000 */

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from files of extension .mustache,
 encoded in UTF8, stored in the provided directory.
 
 For instance:
 
 @code
 // Creates a repository for templates stored in /path/to/templates
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithDirectory:@"/path/to/templates"];
 
 // Returns a template for the file stored in /path/to/templates/profile.mustache
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 A partial tag `{{>partial}}` loads a partial template stored in a file named `partial.mustache`, located in the enclosing template's directory.
 
 You may use the slash `/`, and `..`, in order to navigate the hierarchical file system: `{{>partials/achievements}}`
 would load /path/to/templates/partials/achievements.mustache, if invoked from /path/to/templates/profile.mustache.
 
 When you ask the repository to parse a raw template string, partials are loaded from the base directory:
 
 @code
 // The partial would be loaded from /path/to/templates/partials/achievements.mustache
 GRMustacheTemplate *template = [repository templateFromString:@"{{>partials/achievements}}" error:NULL];
 @endcode
 
 @return a GRMustacheTemplateRepository
 @param path The path of the directory that stores templates.
 */
+ (id)templateRepositoryWithDirectory:(NSString *)path AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from files of provided extension,
 encoded in UTF8, stored in the provided directory.
 
 For instance:
 
 @code
 // Creates a repository for templates of extension `.txt` stored in /path/to/templates
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithDirectory:@"/path/to/templates"
                                                                                        templateExtension:@"txt"];
 
 // Returns a template for the file stored in /path/to/templates/profile.txt
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 A partial tag `{{>partial}}` loads a partial template stored in a file named `partial.txt`, located in the enclosing template's directory.
 
 You may use the slash `/`, and `..`, in order to navigate the hierarchical file system: `{{>partials/achievements}}`
 would load /path/to/templates/partials/achievements.txt, if invoked from /path/to/templates/profile.txt.
 
 When you ask the repository to parse a raw template string, partials are loaded from the base directory:
 
 @code
 // The partial would be loaded from /path/to/templates/partials/achievements.txt
 GRMustacheTemplate *template = [repository templateFromString:@"{{>partials/achievements}}" error:NULL];
 @endcode
 
 @return a GRMustacheTemplateRepository
 @param path The path of the directory that stores templates.
 @param ext The extension of template files.
 */
+ (id)templateRepositoryWithDirectory:(NSString *)path templateExtension:(NSString *)ext AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from files of provided extension,
 encoded in the provided encoding, stored in the provided directory.
 
 For instance:
 
 @code
 // Creates a repository for templates of extension `.txt` stored in /path/to/templates, encoded with NSMacOSRomanStringEncoding:
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithDirectory:@"/path/to/templates"
                                                                                        templateExtension:@"txt"
                                                                                                 encoding:NSMacOSRomanStringEncoding];
 
 // Returns a template for the file stored in /path/to/templates/profile.txt
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 A partial tag `{{>partial}}` loads a partial template stored in a file named `partial.txt`, located in the enclosing template's directory.
 
 You may use the slash `/`, and `..`, in order to navigate the hierarchical file system: `{{>partials/achievements}}`
 would load /path/to/templates/partials/achievements.txt, if invoked from /path/to/templates/profile.txt.
 
 When you ask the repository to parse a raw template string, partials are loaded from the base directory:
 
 @code
 // The partial would be loaded from /path/to/templates/partials/achievements.txt
 GRMustacheTemplate *template = [repository templateFromString:@"{{>partials/achievements}}" error:NULL];
 @endcode
 
 @return a GRMustacheTemplateRepository
 @param path The path of the directory that stores templates.
 @param ext The extension of template files.
 @param encoding The encoding of template files.
 */
+ (id)templateRepositoryWithDirectory:(NSString *)path templateExtension:(NSString *)ext encoding:(NSStringEncoding)encoding AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from resources of extension .mustache,
 encoded in UTF8, stored in the provided bundle.
 
 For instance:
 
 @code
 // Creates a repository for templates stored in the main bundle:
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithBundle:[NSBundle mainBundle]];
 
 // Returns a template for the resource profile.mustache
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 You may provide nil for the bundle parameter: the repository will use the main bundle.
 
 A partial tag `{{>partial}}` loads a partial template from the `partial.mustache` resource in the bundle.
 
 @return a GRMustacheTemplateRepository
 @param bundle The bundle that stores templates as resources.
 */
+ (id)templateRepositoryWithBundle:(NSBundle *)bundle AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from resources of provided extension,
 encoded in UTF8, stored in the provided bundle.
 
 For instance:
 
 @code
 // Creates a repository for templates of extension `.txt` stored in the main bundle:
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithBundle:[NSBundle mainBundle]
                                                                                     templateExtension:@"txt"];
 
 // Returns a template for the resource profile.txt
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 You may provide nil for the bundle parameter: the repository will use the main bundle.
 
 A partial tag `{{>partial}}` loads a partial template from the `partial.txt` resource in the bundle.
 
 @return a GRMustacheTemplateRepository
 @param bundle The bundle that stores templates as resources.
 */
+ (id)templateRepositoryWithBundle:(NSBundle *)bundle templateExtension:(NSString *)ext AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from resources of provided extension,
 encoded in the provided encoding, stored in the provided bundle.
 
 For instance:
 
 @code
 // Creates a repository for templates of extension `.txt` stored in the main bundle, encoded with NSMacOSRomanStringEncoding:
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithBundle:[NSBundle mainBundle]
                                                                                     templateExtension:@"txt"
                                                                                              encoding:NSMacOSRomanStringEncoding];
 
 // Returns a template for the resource profile.txt
 GRMustacheTemplate *template = [repository templateForName:@"profile" error:NULL];
 @endcode
 
 You may provide nil for the bundle parameter: the repository will use the main bundle.
 
 A partial tag `{{>partial}}` loads a partial template from the `partial.txt` resource in the bundle.
 
 @return a GRMustacheTemplateRepository
 @param bundle The bundle that stores templates as resources.
 */
+ (id)templateRepositoryWithBundle:(NSBundle *)bundle templateExtension:(NSString *)ext encoding:(NSStringEncoding)encoding AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository that loads Mustache template strings from a dictionary
 whose keys are template names, and values template strings.
 
 For instance:
 
 @code
 NSDictionary *partialsDictionary = [NSDictionary dictionaryWithObject:@"It works." forKey:@"partial"];
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepositoryWithPartialsDictionary:partialsDictionary];
 
 // Two templates that would render "It works."
 GRMustacheTemplate *template1 = [repository templateForName:@"partial" error:NULL];
 GRMustacheTemplate *template2 = [repository templateFromString:@"{{>partial}}" error:NULL];
 @endcode
 
 @return a GRMustacheTemplateRepository
 @param partialsDictionary A dictionary of template strings for partial names.
 */
+ (id)templateRepositoryWithPartialsDictionary:(NSDictionary *)partialsDictionary AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a GRMustacheTemplateRepository.
 
 Until it is provided with a data source, it is unable to load template by names, and unable to process partial tags such as `{{>partial}}`:
 
 @code
 GRMustacheTemplateRepository *repository = [GRMustacheTemplateRepository templateRepository];
 NSError *error;
 
 // Returns nil, and sets error to an NSError of domain GRMustacheErrorDomain, code GRMustacheErrorCodeTemplateNotFound.
 [repository templateForName:@"foo" error:&error];
 
 // Returns nil, and sets error to an NSError of domain GRMustacheErrorDomain, code GRMustacheErrorCodeTemplateNotFound.
 [repository templateFromString:@"{{>partial}}" error:&error];
 @endcode
 
 It is, however, able to process Mustache template strings without any partial:
 
 @code
 GRMustacheTemplate *template = [repository templateFromString:@"Hello {{name}}!" error:NULL];
 @endcode
 
 You will give it a data source conforming to the GRMustacheTemplateRepositoryDataSource protocol in order to
 load template and partials by name:
 
 @code
 repository.dataSource = ...;
 
 // Returns a template built from the string provided by the dataSource.
 [repository templateForName:@"foo" error:NULL];
 @endcode
 
 @return a GRMustacheTemplateRepository
 */
+ (id)templateRepository AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a template identified by its name.
 
 Depending on the way the repository has been created, the name identifies a URL, a file path, a key in a dictionary,
 or whatever is relevant to the repository's data source.
 
 @return a GRMustacheTemplate
 @param name The template name
 @param outError If there is an error loading or parsing template and partials, upon return contains an NSError object that describes the problem.
 */
- (GRMustacheTemplate *)templateForName:(NSString *)name error:(NSError **)outError AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;

/**
 Returns a template built from the provided Mustache template string.
 
 Depending on the way the repository has been created, partial tags such as `{{>partial}}` load partial templates from URLs, file paths, keys in a dictionary,
 or whatever is relevant to the repository's data source.
 
 @return a GRMustacheTemplate
 @param templateString A Mustache template string
 @param outError If there is an error loading or parsing template and partials, upon return contains an NSError object that describes the problem.
 */
- (GRMustacheTemplate *)templateFromString:(NSString *)templateString error:(NSError **)outError AVAILABLE_GRMUSTACHE_VERSION_4_0_AND_LATER;
@end