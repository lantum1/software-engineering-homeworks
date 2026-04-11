#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include "service/IdentityService.h"
#include "repository/in_memory/InMemoryUserAuthRepository.h"
#include "repository/in_memory/InMemoryUserProfileRepository.h"
#include "repository/in_memory/InMemoryStorage.h"
#include "kafka/StubNotificationPublisher.h"
#include "exceptions/InvalidCredentialsException.h"
#include "exceptions/UserAlreadyExistsException.h"
#include "exceptions/UserNotFoundException.h"

using namespace std;
using namespace maxdisk::identity;

class IdentityServiceTest : public ::testing::Test
{
protected:
    unique_ptr<repository::IUserAuthRepository> userAuthRepository;
    unique_ptr<repository::IUserProfileRepository> userProfileRepository;
    unique_ptr<notification::INotificationPublisher> notificationPublisher;
    unique_ptr<service::IdentityService> identityService;

    const string testUserId = "550e8400-e29b-41d4-a716-446655440000";
    const string testLogin = "test@example.com";
    const string testEmail = "test@example.com";
    const string testPhone = "+79991234567";
    const string testFirstName = "Ivan";
    const string testLastName = "Petrov";
    const string testPassword = "SecurePass123!";

    void SetUp() override
    {
        repository::InMemoryStorage::getInstance().clear();

        userAuthRepository = make_unique<repository::InMemoryUserAuthRepository>();
        userProfileRepository = make_unique<repository::InMemoryUserProfileRepository>();
        notificationPublisher = make_unique<notification::StubNotificationPublisher>();
        identityService = make_unique<service::IdentityService>(
            move(userAuthRepository),
            move(userProfileRepository),
            move(notificationPublisher));
    }

    void TearDown() override
    {
        repository::InMemoryStorage::getInstance().clear();
    }
};

TEST_F(IdentityServiceTest, RegisterUser_Success)
{
    auto response = identityService->registerUser(testPhone, testEmail, testFirstName, testLastName, testPassword);

    EXPECT_FALSE(response.id.empty());
    EXPECT_TRUE(response.id.length() == 36);

    auto user = identityService->searchUserByLogin(testLogin);
    EXPECT_EQ(user.login, testLogin);
    EXPECT_EQ(user.firstName, testFirstName);
    EXPECT_EQ(user.lastName, testLastName);
}

TEST_F(IdentityServiceTest, RegisterUser_DuplicateEmail_Conflict)
{
    identityService->registerUser(testPhone, testEmail, testFirstName, testLastName, testPassword);

    EXPECT_THROW(
        identityService->registerUser("+79999999999", testEmail, "Another", "User", "Pass123"),
        exception::UserAlreadyExistsException);
}

TEST_F(IdentityServiceTest, RegisterUser_DuplicateLogin_Conflict)
{
    identityService->registerUser(testPhone, testEmail, testFirstName, testLastName, testPassword);

    EXPECT_THROW(
        identityService->registerUser("+79998887777", testEmail, "Another", "User", "Pass123"),
        exception::UserAlreadyExistsException);
}

TEST_F(IdentityServiceTest, Login_Success)
{
    identityService->registerUser(testPhone, testEmail, testFirstName, testLastName, testPassword);

    auto response = identityService->authenticate(testLogin, testPassword);

    EXPECT_FALSE(response.userId.empty());
    EXPECT_EQ(response.role, "user");
    EXPECT_EQ(response.email, testEmail);
}

TEST_F(IdentityServiceTest, Login_WrongPassword_Unauthorized)
{
    identityService->registerUser(testPhone, testEmail, testFirstName, testLastName, testPassword);

    EXPECT_THROW(
        identityService->authenticate(testLogin, "WrongPassword"),
        exception::InvalidCredentialsException);
}

TEST_F(IdentityServiceTest, Login_UserNotFound_Unauthorized)
{
    EXPECT_THROW(
        identityService->authenticate("fkrmemofemopfmm", "any_password"),
        exception::InvalidCredentialsException);
}

TEST_F(IdentityServiceTest, SearchUsersByNames_Success)
{
    identityService->registerUser("+79991111111", "ivan.petrov@test.com", "Ivan", "Petrov", "Pass1");
    identityService->registerUser("+79992222222", "ivan.sidorov@test.com", "Ivan", "Sidorov", "Pass2");
    identityService->registerUser("+79993333333", "petr.ivanov@test.com", "Petr", "Ivanov", "Pass3");

    auto response = identityService->searchUsersByNames("*Ivan*", "*");

    EXPECT_EQ(response.users.size(), 2);

    vector<string> foundLogins;
    for (const auto &user : response.users)
    {
        foundLogins.push_back(user.login);
    }
    EXPECT_TRUE(find(foundLogins.begin(), foundLogins.end(), "ivan.petrov@test.com") != foundLogins.end());
    EXPECT_TRUE(find(foundLogins.begin(), foundLogins.end(), "ivan.sidorov@test.com") != foundLogins.end());
}

TEST_F(IdentityServiceTest, SearchUsersByNames_NoResults_EmptyList)
{
    identityService->registerUser(testPhone, testEmail, testFirstName, testLastName, testPassword);

    auto response = identityService->searchUsersByNames("*NonExistent*", "*");

    EXPECT_TRUE(response.users.empty());
}

TEST_F(IdentityServiceTest, SearchUserByLogin_Success)
{
    identityService->registerUser(testPhone, testEmail, testFirstName, testLastName, testPassword);

    auto user = identityService->searchUserByLogin(testLogin);

    EXPECT_EQ(user.login, testLogin);
    EXPECT_EQ(user.firstName, testFirstName);
    EXPECT_EQ(user.lastName, testLastName);
}

TEST_F(IdentityServiceTest, SearchUserByLogin_NotFound)
{
    EXPECT_THROW(
        identityService->searchUserByLogin("fekfmkwlfekw"),
        exception::UserNotFoundException);
}

TEST_F(IdentityServiceTest, SearchWithWildcardMask_Prefix)
{
    identityService->registerUser("+79991111111", "a@test.com", "Alexander", "Smith", "Pass");
    identityService->registerUser("+79992222222", "b@test.com", "Alexandra", "Jones", "Pass");
    identityService->registerUser("+79993333333", "c@test.com", "Bob", "Smith", "Pass");

    auto response = identityService->searchUsersByNames("Alex*", "*");

    EXPECT_EQ(response.users.size(), 2);
}

TEST_F(IdentityServiceTest, SearchWithWildcardMask_Suffix)
{
    identityService->registerUser("+79991111111", "a@test.com", "Ivan", "Petrov", "Pass");
    identityService->registerUser("+79992222222", "b@test.com", "Petr", "Ivanov", "Pass");

    auto response = identityService->searchUsersByNames("*", "*ov");

    EXPECT_EQ(response.users.size(), 2);
}

TEST_F(IdentityServiceTest, SearchWithWildcardMask_Contains)
{
    identityService->registerUser("+79991111111", "a@test.com", "John", "Smith", "Pass");
    identityService->registerUser("+79992222222", "b@test.com", "Johnny", "Doe", "Pass");
    identityService->registerUser("+79993333333", "c@test.com", "Bob", "Johnson", "Pass");

    auto response = identityService->searchUsersByNames("*ohn*", "*");

    EXPECT_EQ(response.users.size(), 2);
}